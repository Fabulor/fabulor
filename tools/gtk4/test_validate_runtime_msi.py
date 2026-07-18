#!/usr/bin/env python3

import unittest
import xml.etree.ElementTree as ET
from unittest import mock

import validate_runtime_msi


def candidate_xml(bin_directory=True):
    if bin_directory:
        payload = """
        <Directory Id="GTK4BINDIR" Name="bin">
          <Component><File Name="gtk-4-1.dll" /></Component>
        </Directory>
        <Component><File Name="runtime-manifest.json" /></Component>
        """
    else:
        payload = """
        <Component><File Name="gtk-4-1.dll" /></Component>
        <Component><File Name="runtime-manifest.json" /></Component>
        """
    return ET.fromstring(
        f"""<Wix xmlns="http://wixtoolset.org/schemas/v4/wxs">
        <Directory Id="GTK4DIR" Name="GTK4">{payload}</Directory>
        </Wix>"""
    )


class ValidateCandidateMsiTests(unittest.TestCase):
    @mock.patch("validate_runtime_msi.subprocess.run")
    def test_decompiler_explicitly_accepts_wix7_eula(self, run):
        run.return_value.returncode = 0

        validate_runtime_msi.decompile_msi(
            "wix.exe", "candidate.msi", "candidate.wxs", "files"
        )

        command = run.call_args.args[0]
        self.assertEqual(
            command[:5],
            ["wix.exe", "msi", "decompile", "-acceptEula", "wix7"],
        )

    def test_matching_directory_layout_passes(self):
        actual = validate_runtime_msi.extract_gtk4_paths(candidate_xml())
        expected = {"bin/gtk-4-1.dll", "runtime-manifest.json"}
        validate_runtime_msi.validate_paths(expected, actual)

    def test_flattened_bin_directory_is_rejected(self):
        actual = validate_runtime_msi.extract_gtk4_paths(
            candidate_xml(bin_directory=False)
        )
        expected = {"bin/gtk-4-1.dll", "runtime-manifest.json"}
        with self.assertRaisesRegex(
                validate_runtime_msi.CandidateMsiError, "differs"):
            validate_runtime_msi.validate_paths(expected, actual)

    def test_unexpected_payload_is_rejected(self):
        actual = validate_runtime_msi.extract_gtk4_paths(candidate_xml())
        actual.add("bin/debug.pdb")
        expected = {"bin/gtk-4-1.dll", "runtime-manifest.json"}
        with self.assertRaisesRegex(
                validate_runtime_msi.CandidateMsiError, "unexpected"):
            validate_runtime_msi.validate_paths(expected, actual)

    def test_duplicate_installed_path_is_rejected(self):
        root = candidate_xml()
        namespace = "{http://wixtoolset.org/schemas/v4/wxs}"
        bin_directory = next(
            node for node in root.findall(f".//{namespace}Directory")
            if node.get("Id") == "GTK4BINDIR"
        )
        component = ET.SubElement(bin_directory, f"{namespace}Component")
        ET.SubElement(component, f"{namespace}File", Name="gtk-4-1.dll")
        with self.assertRaisesRegex(
                validate_runtime_msi.CandidateMsiError, "duplicate"):
            validate_runtime_msi.extract_gtk4_paths(root)


if __name__ == "__main__":
    unittest.main()
