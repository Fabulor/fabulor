import pathlib
import json
import unittest
import xml.etree.ElementTree as ET


ROOT = pathlib.Path(__file__).resolve().parents[2]
INSTALLER = ROOT / "installer"
PROPS = ROOT / "win32" / "zoitechat.props"
SOLUTION = ROOT / "win32" / "zoitechat.sln"
NATIVE_EXTENSIONS = ROOT / "tools" / "gtk4" / "gtk4-native-extensions.proj"
GTK_COMPAT = ROOT / "src" / "fe-gtk" / "gtk-compat.h"
GTK4_OPERATIONAL_LIST_SOURCES = (
    "addon-list.c",
    "ban-list.c",
    "channel-list.c",
    "channel-model.c",
    "channel-model.h",
    "channel-tree-view.c",
    "dcc-chat-list.c",
    "dcc-transfer-list.c",
    "editable-list.c",
    "gtk4-list-models.h",
    "ignore-list.c",
    "key-binding-list.c",
    "notify-list.c",
    "preferences-category-list.c",
    "print-event-list.c",
    "server-entry-list.c",
    "server-network-list.c",
    "sound-event-list.c",
    "url-list.c",
    "user-list-model.c",
    "user-list-model.h",
    "user-list-view.c",
)
GTK4_THEME_SOURCES = (
    "theme-access.c",
    "theme-appearance-monitor-gtk4.c",
    "theme-css.c",
    "theme-gtk4-controller.c",
    "theme-gtk4.c",
    "theme-manager.c",
    "theme-manager.h",
    "theme-preferences-gtk4.c",
    "theme-preferences.c",
)
VCPKG_CONFIGURATION = ROOT / "tools" / "windows-deps" / "vcpkg-configuration.json"
VCPKG_MANIFEST = ROOT / "tools" / "windows-deps" / "vcpkg.json"
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
        self.assertNotIn('= "lua"', solution)
        self.assertIn('= "fabulor-launcher"', solution)

    def test_frontend_compatibility_header_is_gtk4_only(self):
        source = GTK_COMPAT.read_text(encoding="utf-8")

        for token in (
            "GTK_MAJOR_VERSION",
            "gtk_bin_",
            "gtk_box_pack_",
            "gtk_container_",
            "gtk_dialog_run",
            "gtk_selection_",
            "gtk_widget_destroy",
            "gtk_widget_show_all",
        ):
            self.assertNotRegex(source, rf"\b{token}")
        for type_name in ("GtkButtonBox", "GtkIconSize", "GdkEventButton"):
            self.assertNotRegex(source, rf"\b{type_name}\b")

    def test_operational_list_sources_are_gtk4_only(self):
        frontend = ROOT / "src" / "fe-gtk"
        retired_types = (
            "GtkTreeView",
            "GtkListStore",
            "GtkTreeStore",
            "GtkCellRenderer",
            "GtkTreeSelection",
        )
        retired_functions = (
            "gtk_tree_view_",
            "gtk_list_store_",
            "gtk_tree_store_",
            "gtk_cell_renderer_",
            "gtk_tree_selection_",
        )

        for name in GTK4_OPERATIONAL_LIST_SOURCES:
            source = (frontend / name).read_text(encoding="utf-8")
            with self.subTest(source=name):
                self.assertNotIn("GTK_MAJOR_VERSION", source)
                for type_name in retired_types:
                    self.assertNotRegex(source, rf"\b{type_name}\b")
                for function in retired_functions:
                    self.assertNotRegex(source, rf"\b{function}")

    def test_theme_sources_are_gtk4_only(self):
        theme = ROOT / "src" / "fe-gtk" / "theme"
        retired_functions = (
            "gdk_screen_get_default",
            "gtk_css_provider_load_from_data",
            "gtk_style_context_add_provider_for_screen",
            "gtk_style_context_get_background_color",
            "gtk_style_context_get_border_color",
            "gtk_style_context_get_color",
            "gtk_style_context_remove_provider_for_screen",
            "gtk_widget_destroy",
        )

        for name in GTK4_THEME_SOURCES:
            source = (theme / name).read_text(encoding="utf-8")
            with self.subTest(source=name):
                self.assertNotIn("GTK_MAJOR_VERSION", source)
                for function in retired_functions:
                    self.assertNotRegex(source, rf"\b{function}\b")

    def test_transitional_windows_staging_is_removed(self):
        props = PROPS.read_text(encoding="utf-8")
        extensions = NATIVE_EXTENSIONS.read_text(encoding="utf-8")

        for token in ("YourDepsPath", "GendefPath", "PerlEnabled", "LuaEnabled",
                      "ArchiveLib", "HAVE_LIBARCHIVE", "InstallerEnabled",
                      "IsccPath"):
            self.assertNotIn(token, props)
        self.assertNotIn("plugins\\lua\\lua.vcxproj", extensions)
        self.assertFalse((ROOT / "win32" / "copy" / "copy.vcxproj").exists())
        legacy_installer = ROOT / "win32" / "installer"
        self.assertFalse(legacy_installer.exists() and any(legacy_installer.iterdir()))

    def test_windows_support_dependencies_are_pinned(self):
        manifest = json.loads(VCPKG_MANIFEST.read_text(encoding="utf-8"))
        configuration = json.loads(VCPKG_CONFIGURATION.read_text(encoding="utf-8"))

        self.assertEqual(manifest["dependencies"], ["openssl"])
        self.assertRegex(
            configuration["default-registry"]["baseline"], r"^[0-9a-f]{40}$"
        )


if __name__ == "__main__":
    unittest.main()
