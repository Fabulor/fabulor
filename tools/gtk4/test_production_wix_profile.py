import pathlib
import unittest
import xml.etree.ElementTree as ET


ROOT = pathlib.Path(__file__).resolve().parents[2]
INSTALLER = ROOT / "installer"
WIX_NS = {"w": "http://wixtoolset.org/schemas/v4/wxs"}


class ProductionWixProfileTests(unittest.TestCase):
    def test_production_product_keeps_upgrade_identity(self):
        root = ET.parse(INSTALLER / "ProductGtk4.wxs").getroot()
        package = root.find("w:Package", WIX_NS)

        self.assertIsNotNone(package)
        self.assertEqual(package.get("Name"), "Fabulor")
        self.assertEqual(
            package.get("UpgradeCode"),
            "8F6C0C7E-9A4D-4E4C-9F8C-2B6F5A4E9C11",
        )
        install = root.find(".//w:Directory[@Id='INSTALLFOLDER']", WIX_NS)
        self.assertEqual(install.get("Name"), "Fabulor")

    def test_production_product_has_no_legacy_gtk_payload(self):
        source = (INSTALLER / "ProductGtk4.wxs").read_text(encoding="utf-8")

        self.assertNotIn("LegacyGtkCompatibilityFeature", source)
        self.assertNotIn("LegacyGtkRootRuntimeComponents", source)
        self.assertNotIn("SHAREGTK3DIR", source)
        self.assertIn('ComponentGroupRef Id="GTK4Components"', source)

    def test_default_project_profile_selects_only_gtk4_components(self):
        root = ET.parse(INSTALLER / "Fabulor.wixproj").getroot()
        compile_items = root.findall(".//Compile")
        default_condition = (
            "'$(Gtk4FrontendCandidate)'!='true' and "
            "'$(LegacyGtk3Frontend)'!='true'"
        )
        default_sources = {
            item.get("Include")
            for item in compile_items
            if item.get("Condition") == default_condition
        }

        self.assertIn("ProductGtk4.wxs", default_sources)
        component_sources = next(
            source for source in default_sources
            if source and source.startswith("Components\\Config.wxs")
        )
        self.assertIn("Components\\CoreGtk4.wxs", component_sources)
        self.assertIn("Components\\GTK4Allowlist.wxs", component_sources)
        self.assertNotIn("Components\\Core.wxs", component_sources)
        self.assertNotIn("LegacyGtkRootRuntime.wxs", component_sources)

    def test_legacy_profile_is_explicit(self):
        root = ET.parse(INSTALLER / "Fabulor.wixproj").getroot()
        properties = {
            child.tag.rsplit("}", 1)[-1]: child.text
            for group in root.findall("PropertyGroup")
            for child in group
        }

        self.assertEqual(properties.get("LegacyGtk3Frontend"), "false")


if __name__ == "__main__":
    unittest.main()
