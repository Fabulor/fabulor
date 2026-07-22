import pathlib
import unittest
import xml.etree.ElementTree as ET


ROOT = pathlib.Path(__file__).resolve().parents[2]
INSTALLER = ROOT / "installer"
PROPS = ROOT / "win32" / "zoitechat.props"
SOLUTION = ROOT / "win32" / "zoitechat.sln"
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

    def test_project_has_one_unconditional_gtk4_product_graph(self):
        root = ET.parse(INSTALLER / "Fabulor.wixproj").getroot()
        compile_items = root.findall(".//Compile")
        self.assertEqual(len(compile_items), 2)
        self.assertTrue(all(item.get("Condition") is None for item in compile_items))
        default_sources = {item.get("Include") for item in compile_items}
        self.assertIn("ProductGtk4.wxs", default_sources)
        component_sources = next(
            source for source in default_sources
            if source and source.startswith("Components\\Config.wxs")
        )
        self.assertIn("Components\\CoreGtk4.wxs", component_sources)
        self.assertIn("Components\\GTK4Allowlist.wxs", component_sources)
        self.assertNotIn("Components\\Core.wxs", component_sources)
        self.assertNotIn("LegacyGtkRootRuntime.wxs", component_sources)

    def test_obsolete_product_graphs_are_removed(self):
        obsolete = {
            "Product.wxs",
            "ProductGtk4Candidate.wxs",
            "Components/Core.wxs",
            "Components/CoreGtk4Candidate.wxs",
            "Components/LegacyGtkRootRuntime.wxs",
            "Components/NativeGtk4Candidate.wxs",
            "Components/PluginHostsGtk4Candidate.wxs",
        }
        self.assertFalse([path for path in obsolete if (INSTALLER / path).exists()])

    def test_project_exposes_no_frontend_profile_switch(self):
        source = (INSTALLER / "Fabulor.wixproj").read_text(encoding="utf-8")
        self.assertNotIn("LegacyGtk3Frontend", source)
        self.assertNotIn("Gtk4FrontendCandidate", source)
        self.assertIn("gtk4-runtime-production-root", source)
        self.assertIn("gtk4-plugin-host-production-root", source)

    def test_msvc_profile_is_gtk4_only(self):
        props = PROPS.read_text(encoding="utf-8")
        solution = SOLUTION.read_text(encoding="utf-8-sig")

        self.assertIn("<FabulorGtkMajor>4</FabulorGtkMajor>", props)
        self.assertIn("Fabulor supports only the GTK4 frontend build profile", props)
        self.assertNotIn("Gtk3Lib", props)
        self.assertNotIn("Gdk3Lib", props)
        self.assertNotIn('= "copy", "copy\\copy.vcxproj"', solution)
        self.assertNotIn('= "fe-text"', solution)
        self.assertIn('= "fabulor-launcher"', solution)


if __name__ == "__main__":
    unittest.main()
