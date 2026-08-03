#!/usr/bin/env python3

import pathlib
import tempfile
import unittest
import xml.etree.ElementTree as ET
from unittest import mock

import validate_production_bundle


TEST_TEMP_ROOT = pathlib.Path(__file__).resolve().parents[2] / "build" / "test-temp"


BA_XML = """\
<BootstrapperApplicationData xmlns="http://wixtoolset.org/schemas/v4/BootstrapperApplicationData">
  <WixBundleProperties BundleId="Fabulor.Setup.Bundle" DisplayName="Fabulor Setup"
    UpgradeCode="{D9F4A5C2-7F3B-4F9E-9A21-3C8F6B7E4A10}" Scope="perMachine" />
  <WixPackageProperties Package="FabulorMsi" DisplayName="Fabulor"
    PackageType="Msi" Compressed="yes" ProductCode="{PRODUCT-CODE}"
    UpgradeCode="{8F6C0C7E-9A4D-4E4C-9F8C-2B6F5A4E9C11}" Version="1.0.3"
    PackageSize="4" />
  <WixPayloadProperties Package="FabulorMsi" Payload="FabulorMsi"
    Container="WixAttachedContainer" Name="Fabulor.msi" Size="4" />
</BootstrapperApplicationData>
"""

BURN_XML = """\
<BurnManifest xmlns="http://wixtoolset.org/schemas/v4/2008/Burn">
  <RelatedBundle Code="{D9F4A5C2-7F3B-4F9E-9A21-3C8F6B7E4A10}" Action="Upgrade" />
  <Payload Id="FabulorMsi" FilePath="Fabulor.msi" Packaging="embedded"
    Container="WixAttachedContainer" />
  <Registration BundleId="Fabulor.Setup.Bundle" Version="1.0.3"
    Scope="perMachine"
    PrimaryUpgradeCode="{D9F4A5C2-7F3B-4F9E-9A21-3C8F6B7E4A10}">
    <Arp Publisher="Fabulor" />
  </Registration>
  <Chain>
    <MsiPackage Id="FabulorMsi" ProductCode="{PRODUCT-CODE}" Version="1.0.3"
      UpgradeCode="{8F6C0C7E-9A4D-4E4C-9F8C-2B6F5A4E9C11}"
      Scope="perMachine">
      <PayloadRef Id="FabulorMsi" />
    </MsiPackage>
  </Chain>
</BurnManifest>
"""


class ValidateProductionBundleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        TEST_TEMP_ROOT.mkdir(parents=True, exist_ok=True)

    @mock.patch("validate_production_bundle.subprocess.run")
    def test_extraction_uses_separate_chain_and_ba_roots(self, run):
        run.return_value.returncode = 0

        validate_production_bundle.extract_bundle(
            "wix.exe", "FabulorSetup.exe", "chain", "ba"
        )

        self.assertEqual(
            run.call_args.args[0],
            [
                "wix.exe",
                "burn",
                "extract",
                "-acceptEula",
                "wix7",
                "FabulorSetup.exe",
                "-o",
                "chain",
                "-oba",
                "ba",
            ],
        )

    def test_matching_bundle_identity_passes(self):
        ba_root = ET.fromstring(BA_XML)
        product_code, size = (
            validate_production_bundle.validate_bootstrapper_data(
                ba_root, "1.0.3"
            )
        )
        self.assertEqual(product_code, "{PRODUCT-CODE}")
        self.assertEqual(size, 4)

        burn_root = ET.fromstring(BURN_XML)
        validate_production_bundle.validate_burn_manifest(
            burn_root, "1.0.3", product_code
        )

    def test_bundle_upgrade_code_change_is_rejected(self):
        root = ET.fromstring(
            BA_XML.replace(
                "D9F4A5C2-7F3B-4F9E-9A21-3C8F6B7E4A10",
                "00000000-0000-0000-0000-000000000000",
                1,
            )
        )
        with self.assertRaisesRegex(
            validate_production_bundle.ProductionBundleError,
            "bundle upgrade code",
        ):
            validate_production_bundle.validate_bootstrapper_data(root, "1.0.3")

    def test_unexpected_bootstrapper_file_is_rejected(self):
        with tempfile.TemporaryDirectory(dir=TEST_TEMP_ROOT) as temporary:
            temporary = pathlib.Path(temporary)
            ba_root = temporary / "ba"
            chain_root = temporary / "chain"
            for relative in validate_production_bundle.EXPECTED_BA_FILES:
                path = ba_root / pathlib.PurePosixPath(relative)
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b"")
            embedded = chain_root / "WixAttachedContainer" / "Fabulor.msi"
            embedded.parent.mkdir(parents=True)
            embedded.write_bytes(b"msi")
            (ba_root / "unexpected.dll").write_bytes(b"")

            with self.assertRaisesRegex(
                validate_production_bundle.ProductionBundleError,
                "unexpected",
            ):
                validate_production_bundle.validate_extracted_files(
                    ba_root, chain_root
                )

    def test_embedded_msi_mismatch_is_rejected(self):
        with tempfile.TemporaryDirectory(dir=TEST_TEMP_ROOT) as temporary:
            temporary = pathlib.Path(temporary)
            published = temporary / "Fabulor.msi"
            embedded = temporary / "embedded.msi"
            published.write_bytes(b"same")
            embedded.write_bytes(b"else")

            with self.assertRaisesRegex(
                validate_production_bundle.ProductionBundleError,
                "does not match",
            ):
                validate_production_bundle.validate_embedded_msi(
                    published, embedded, 4
                )

    def test_version_comes_from_installer_properties(self):
        with tempfile.TemporaryDirectory(dir=TEST_TEMP_ROOT) as temporary:
            props = pathlib.Path(temporary) / "Directory.Build.props"
            props.write_text(
                "<Project><PropertyGroup><FabulorSemVer>1.2.3"
                "</FabulorSemVer></PropertyGroup></Project>",
                encoding="utf-8",
            )
            self.assertEqual(
                validate_production_bundle.load_expected_version(props),
                "1.2.3",
            )


if __name__ == "__main__":
    unittest.main()
