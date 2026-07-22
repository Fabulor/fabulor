#!/usr/bin/env python3

import pathlib
import sys
import unittest
import xml.etree.ElementTree as ET
from unittest import mock


sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import validate_frontend_candidate_msi as candidate


NS = candidate.validate_runtime_msi.WIX_NAMESPACE


def xml_document(name=candidate.PRODUCT_NAME,
                 upgrade_code=candidate.UPGRADE_CODE,
                 extra_file=""):
    return ET.fromstring(
        f"""<Wix xmlns=\"{NS}\">
  <Package Name=\"{name}\" UpgradeCode=\"{upgrade_code}\">
    <Directory Id=\"INSTALLFOLDER\" Name=\"{candidate.INSTALL_FOLDER_NAME}\">
      <Component><File Name=\"fabulor.exe\" Source=\"SourceDir\\fabulor.exe\" />{extra_file}</Component>
      <Directory Id=\"RUNTIMEDIR\" Name=\"Runtime\">
        <Directory Id=\"GTK4DIR\" Name=\"GTK4\">
          <Component><File Name=\"runtime-manifest.json\" Source=\"SourceDir\\Runtime\\GTK4\\runtime-manifest.json\" /></Component>
        </Directory>
      </Directory>
    </Directory>
  </Package>
</Wix>"""
    )


class FrontendCandidateMsiTests(unittest.TestCase):
    def test_identity_and_payload_paths_are_accepted(self):
        root = xml_document()
        candidate.validate_identity(root)
        sources = candidate.extract_sources(candidate.find_install_folder(root))
        self.assertEqual(
            set(sources),
            {"fabulor.exe", "Runtime/GTK4/runtime-manifest.json"},
        )

    def test_wrong_product_identity_is_rejected(self):
        with self.assertRaises(candidate.FrontendCandidateMsiError):
            candidate.validate_identity(xml_document(name="Fabulor"))
        with self.assertRaises(candidate.FrontendCandidateMsiError):
            candidate.validate_identity(xml_document(upgrade_code="BAD-CODE"))

    def test_shortcut_side_effect_is_rejected(self):
        root = xml_document()
        package = root.find("w:Package", candidate.NS)
        ET.SubElement(package, f"{{{NS}}}Shortcut", {"Id": "CandidateShortcut"})
        with self.assertRaisesRegex(
                candidate.FrontendCandidateMsiError, "forbidden Shortcut"):
            candidate.validate_identity(root)

    def test_unexpected_gtk3_file_is_rejected(self):
        root = xml_document(
            extra_file=(
                '<File Name="libgtk-3-0.dll" '
                'Source="SourceDir\\libgtk-3-0.dll" />'
            )
        )
        actual = set(candidate.extract_sources(candidate.find_install_folder(root)))
        with self.assertRaisesRegex(
                candidate.FrontendCandidateMsiError, "unexpected"):
            candidate.validate_paths(
                {"fabulor.exe", "Runtime/GTK4/runtime-manifest.json"}, actual
            )

    def test_content_hash_mismatch_is_rejected(self):
        root = xml_document()
        sources = candidate.extract_sources(candidate.find_install_folder(root))
        expected = {
            "fabulor.exe": (8, "0" * 64),
            "Runtime/GTK4/runtime-manifest.json": (8, "1" * 64),
        }
        with mock.patch.object(
                candidate, "file_content", return_value=(8, "2" * 64)):
            with self.assertRaisesRegex(
                    candidate.FrontendCandidateMsiError, "content differs"):
                candidate.validate_content(expected, sources, pathlib.Path("files"))


if __name__ == "__main__":
    unittest.main()
