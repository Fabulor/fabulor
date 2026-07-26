import pathlib
import json
import re
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
GTK4_WINDOW_HELPER_SOURCES = (
    "file-chooser-path.c",
    "window-geometry.c",
    "window-state.c",
)
GTK4_SPELL_INPUT_SOURCES = (
    "emoji-picker.c",
    "sexy-spell-entry.c",
    "spell-entry-widget.c",
)
GTK4_TRANSCRIPT_HELPER_SOURCES = (
    "xtext-accessible.c",
    "xtext-accessible.h",
    "xtext-geometry.c",
    "xtext-render-target.c",
    "xtext-render-target.h",
    "xtext-selection.c",
    "xtext-widget-class.c",
    "xtext.h",
)
GTK4_TRANSCRIPT_RENDERER = ROOT / "src" / "fe-gtk" / "xtext.c"
GTK4_TRAY_SOURCES = (
    "plugin-tray.c",
    "tray-menu-presenter-gtk4.c",
)
FRONTEND_MESON = ROOT / "src" / "fe-gtk" / "meson.build"
GTK4_APPLICATION_SOURCE = ROOT / "src" / "fe-gtk" / "fe-gtk.c"
GTK4_SERVER_LIST_SOURCE = ROOT / "src" / "fe-gtk" / "servlistgui.c"
GTK4_CHANNEL_BAN_DIALOG_SOURCES = (
    "banlist.c",
    "chanlist.c",
)
GTK4_PREFERENCES_JOIN_SOURCES = (
    "joind.c",
    "setup.c",
)
GTK4_SMALL_HELPER_SOURCES = (
    "chanview.c",
    "context-menu-presenter-gtk4.c",
    "fkeys.h",
    "gtkutil.c",
    "pixmaps.c",
)
GTK4_MAIN_WINDOW_SOURCES = (
    "maingui.c",
    "maingui.h",
)
GTK4_MENU_SOURCES = (
    "menu.c",
    "menu.h",
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

    def test_production_product_installs_complete_enchant_payload(self):
        product = (INSTALLER / "ProductGtk4.wxs").read_text(encoding="utf-8")
        enchant = (
            INSTALLER / "Components" / "EnchantGtk4.wxs"
        ).read_text(encoding="utf-8")

        for group in (
            "Enchant2CoreComponents",
            "Enchant2ProviderComponents",
            "Enchant2DataComponents",
        ):
            self.assertIn(f'ComponentGroupRef Id="{group}"', product)
        self.assertIn(
            r'$(var.Gtk4EnchantRoot)\libenchant-2-2.dll', enchant
        )
        self.assertIn(
            r'$(var.Gtk4EnchantRoot)\lib\enchant-2\enchant_winspell.dll',
            enchant,
        )
        self.assertIn(
            r'$(var.Gtk4EnchantRoot)\share\enchant-2\enchant.ordering',
            enchant,
        )

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

    def test_window_helper_sources_are_gtk4_only(self):
        frontend = ROOT / "src" / "fe-gtk"
        retired_tokens = (
            "GdkEventConfigure",
            "GdkEventWindowState",
            "GdkWindowState",
            "gtk_file_chooser_get_current_folder_file",
            "gtk_file_chooser_set_current_folder_file",
            "gtk_file_chooser_set_do_overwrite_confirmation",
            "gtk_file_chooser_set_local_only",
            "gtk_window_deiconify",
            "gtk_window_get_position",
            "gtk_window_get_size",
            "gtk_widget_get_window",
        )

        for name in GTK4_WINDOW_HELPER_SOURCES:
            source = (frontend / name).read_text(encoding="utf-8")
            with self.subTest(source=name):
                self.assertNotIn("GTK_MAJOR_VERSION", source)
                for token in retired_tokens:
                    self.assertNotRegex(source, rf"\b{token}\b")

    def test_spell_input_sources_are_gtk4_only(self):
        frontend = ROOT / "src" / "fe-gtk"
        retired_tokens = (
            "GtkMenu",
            "gtk_container_add",
            "gtk_css_provider_load_from_data",
            "gtk_entry_get_layout",
            "gtk_entry_get_layout_offsets",
            "gtk_entry_get_text",
            "gtk_menu_item_",
            "gtk_menu_new",
            "gtk_menu_shell_",
            "gtk_popover_set_modal",
            "gtk_widget_destroy",
            "gtk_widget_show_all",
        )

        for name in GTK4_SPELL_INPUT_SOURCES:
            source = (frontend / name).read_text(encoding="utf-8")
            with self.subTest(source=name):
                self.assertNotIn("GTK_MAJOR_VERSION", source)
                for token in retired_tokens:
                    self.assertNotRegex(source, rf"\b{token}")

    def test_transcript_helper_sources_are_gtk4_only(self):
        frontend = ROOT / "src" / "fe-gtk"
        retired_tokens = (
            "AtkObject",
            "GdkEventSelection",
            "GdkWindow",
            "GtkAllocation",
            "GtkClipboard",
            "GtkSelectionData",
            "GtkTargetEntry",
            "atk_object_set_name",
            "gdk_cairo_create",
            "gdk_cairo_get_clip_rectangle",
            "gtk_clipboard_",
            "gtk_selection_",
            "gtk_widget_get_accessible",
            "gtk_widget_get_allocated_height",
            "gtk_widget_get_allocated_width",
            "gtk_widget_get_allocation",
            "gtk_widget_get_clipboard",
            "gtk_widget_get_window",
        )

        for name in GTK4_TRANSCRIPT_HELPER_SOURCES:
            source = (frontend / name).read_text(encoding="utf-8")
            with self.subTest(source=name):
                self.assertNotIn("GTK_MAJOR_VERSION", source)
                for token in retired_tokens:
                    self.assertNotRegex(source, rf"\b{token}")

    def test_transcript_renderer_is_gtk4_only(self):
        source = GTK4_TRANSCRIPT_RENDERER.read_text(encoding="utf-8")
        retired_tokens = (
            "gdk_cairo_set_source_window",
            "gdk_cursor_new_for_display",
            "gdk_device_",
            "gdk_window_",
            "gtk_grab_",
            "gtk_widget_get_events",
            "gtk_widget_get_parent_window",
            "gtk_widget_get_window",
            "gtk_widget_set_allocation",
            "gtk_widget_set_realized",
            "gtk_widget_set_window",
            "gtk_xtext_clear_background",
            "gtk_xtext_get_pointer",
            "gtk_xtext_style_updated",
            "xtext_surface_from_window",
        )

        self.assertNotIn("GTK_MAJOR_VERSION", source)
        for token in retired_tokens:
            self.assertNotRegex(source, rf"\b{token}")
        self.assertIn("xtext_begin_draw", source)
        self.assertNotIn("xtext_create_context", source)
        self.assertEqual(
            source.count("fabulor_xtext_render_target_create_context"), 1
        )

    def test_tray_sources_are_gtk4_only(self):
        frontend = ROOT / "src" / "fe-gtk"
        retired_tokens = (
            "AppIndicator",
            "GtkCheckMenuItem",
            "GtkStatusIcon",
            "HAVE_APPINDICATOR",
            "HAVE_AYATANA_APPINDICATOR",
            "HAVE_LEGACY_STATUS_ICON_BACKEND",
            "app_indicator_",
            "gtk_check_menu_item_",
            "gtk_menu_item_",
            "gtk_menu_shell_",
            "gtk_status_icon_",
            "gtk_widget_destroy",
        )

        for name in GTK4_TRAY_SOURCES:
            source = (frontend / name).read_text(encoding="utf-8")
            with self.subTest(source=name):
                self.assertNotIn("GTK_MAJOR_VERSION", source)
                for token in retired_tokens:
                    self.assertNotRegex(source, rf"\b{token}")
        plugin_source = (frontend / "plugin-tray.c").read_text(encoding="utf-8")
        self.assertIn("environment.toolkit_major = 4;", plugin_source)

        meson = FRONTEND_MESON.read_text(encoding="utf-8")
        self.assertNotRegex(meson, r"\b(?:ayatana-)?appindicator")

    def test_application_lifecycle_is_gtk4_only(self):
        source = GTK4_APPLICATION_SOURCE.read_text(encoding="utf-8")

        self.assertNotIn("GTK_MAJOR_VERSION", source)
        self.assertNotRegex(source, r"\bgtk_get_option_group\b")
        self.assertNotRegex(source, r"\bgtk_main(?:_quit)?\b")
        self.assertNotRegex(source, r"\bgtk_init\s*\(&")
        self.assertIn("fabulor_application_main_loop_run", source)
        self.assertIn("fabulor_application_main_loop_request_quit", source)
        self.assertIn('"Runtime", "GTK4"', source)

    def test_server_list_lifecycle_is_gtk4_only(self):
        source = GTK4_SERVER_LIST_SOURCE.read_text(encoding="utf-8")
        retired_tokens = (
            "GdkEventAny",
            "delete-event",
            "parent_destroy_handler",
            "servlist_cert_import_parent_destroy_cb",
            "servlist_edit_destroy_cb",
            "servlist_editwin_delete_cb",
            "servlist_window_destroy_cb",
        )

        self.assertNotIn("GTK_MAJOR_VERSION", source)
        for token in retired_tokens:
            self.assertNotRegex(source, rf"\b{token}")
        self.assertIn('"close-request"', source)
        self.assertIn("g_object_weak_ref", source)
        self.assertNotRegex(source, r"\bgtk_toggle_button_(?:get|set)_active\b")
        self.assertIn("fabulor_gtk_check_button_get_active", source)
        self.assertIn("fabulor_gtk_check_button_set_active", source)

    def test_channel_and_ban_dialogs_are_gtk4_only(self):
        frontend = ROOT / "src" / "fe-gtk"
        retired_tokens = (
            "chanlist_copychannel",
            "chanlist_copytopic",
            "chanlist_icon_menu_item",
            "chanlist_menu_destroy",
            "gtk_box_pack_start",
            "gtk_container_add",
            "gtk_menu_",
            "gtk_menu_shell_",
            "gtk_widget_destroy",
            "gtk_widget_show_all",
        )

        for name in GTK4_CHANNEL_BAN_DIALOG_SOURCES:
            source = (frontend / name).read_text(encoding="utf-8")
            with self.subTest(source=name):
                self.assertNotIn("GTK_MAJOR_VERSION", source)
                for token in retired_tokens:
                    self.assertNotRegex(source, rf"\b{token}")

    def test_preferences_and_join_dialogs_are_gtk4_only(self):
        frontend = ROOT / "src" / "fe-gtk"
        retired_tokens = (
            "GTK_BIN",
            "joind_destroy_cb",
            "setup_close_cb",
            "gtk_bin_get_child",
            "gtk_viewport_set_shadow_type",
        )

        for name in GTK4_PREFERENCES_JOIN_SOURCES:
            source = (frontend / name).read_text(encoding="utf-8")
            with self.subTest(source=name):
                self.assertNotIn("GTK_MAJOR_VERSION", source)
                for token in retired_tokens:
                    self.assertNotRegex(source, rf"\b{token}")

        join_source = (frontend / "joind.c").read_text(encoding="utf-8")
        setup_source = (frontend / "setup.c").read_text(encoding="utf-8")
        self.assertIn("joind_finalized_cb", join_source)
        self.assertIn("setup_fontchooser_finalized_cb", setup_source)
        self.assertIn("setup_window_finalized_cb", setup_source)
        self.assertIn("g_object_weak_ref", join_source)
        self.assertIn("g_object_weak_ref", setup_source)

    def test_channel_view_and_small_helpers_are_gtk4_only(self):
        frontend = ROOT / "src" / "fe-gtk"
        retired_tokens = (
            "GDK_MOD1_MASK",
            "GdkScreen",
            "GTK_ICON_LOOKUP_FORCE_SIZE",
            "HAVE_APPINDICATOR",
            "HAVE_AYATANA_APPINDICATOR",
            "chanview_box_destroy_cb",
            "gdk_x11_",
            "gtk_icon_theme_get_default",
            "gtk_icon_theme_load_icon",
            "gtkutil_treemodel_string_to_iter",
            "gtkutil_treeview_get_selected",
            "gtkutil_treeview_new",
            "gtk_window_get_screen",
        )

        for name in GTK4_SMALL_HELPER_SOURCES:
            source = (frontend / name).read_text(encoding="utf-8")
            with self.subTest(source=name):
                self.assertNotIn("GTK_MAJOR_VERSION", source)
                for token in retired_tokens:
                    self.assertNotRegex(source, rf"\b{token}")

        channel_view = (frontend / "chanview.c").read_text(encoding="utf-8")
        icon_source = (frontend / "pixmaps.c").read_text(encoding="utf-8")
        utility_header = (frontend / "gtkutil.h").read_text(encoding="utf-8")
        key_header = (frontend / "fkeys.h").read_text(encoding="utf-8")
        key_source = (frontend / "fkeys.c").read_text(encoding="utf-8")
        text_editor = (frontend / "textgui.c").read_text(encoding="utf-8")
        theme_preferences = (
            frontend / "theme" / "theme-preferences.c"
        ).read_text(encoding="utf-8")
        theme_runtime = (
            frontend / "theme" / "theme-runtime.c"
        ).read_text(encoding="utf-8")
        self.assertIn("chanview_box_finalized_cb", channel_view)
        self.assertIn("g_object_weak_ref", channel_view)
        self.assertIn("gtk_icon_paintable_get_file", icon_source)
        self.assertIn("GDK_ALT_MASK", key_header)
        self.assertNotIn("gtkutil_treeview_", utility_header)
        self.assertNotIn('}", -1, NULL);', key_source)
        self.assertNotRegex(
            text_editor, r"\bgtk_scrolled_window_new\s*\(\s*(?:NULL|0)"
        )
        self.assertNotRegex(
            theme_preferences, r"\bgtk_scrolled_window_new\s*\(\s*(?:NULL|0)"
        )
        self.assertIn("65535.0f", theme_runtime)
        self.assertIn("1.0f }", theme_runtime)

    def test_main_window_sources_are_gtk4_only(self):
        frontend = ROOT / "src" / "fe-gtk"
        retired_tokens = (
            "GdkEvent",
            "GdkWindow",
            "GtkAccelGroup",
            "GtkCheckMenuItem",
            "GtkMenu",
            "gdk_event_free",
            "gdk_window_",
            "gtk_accel_group_new",
            "gtk_bin_get_child",
            "gtk_check_menu_item_get_active",
            "gtk_container_get_children",
            "gtk_drag_dest_set",
            "gtk_get_current_event",
            "gtk_menu_",
            "gtk_paned_get_child1",
            "gtk_paned_get_child2",
            "gtk_widget_destroy",
            "gtk_widget_get_window",
            "gtk_widget_queue_draw_area",
            "gtk_window_add_accel_group",
            "mg_create_icon_item",
            "mg_submenu",
        )

        for name in GTK4_MAIN_WINDOW_SOURCES:
            source = (frontend / name).read_text(encoding="utf-8")
            with self.subTest(source=name):
                self.assertNotIn("GTK_MAJOR_VERSION", source)
                for token in retired_tokens:
                    self.assertNotRegex(source, rf"\b{token}")

        main_source = (frontend / "maingui.c").read_text(encoding="utf-8")
        compat_source = (frontend / "gtk-compat.h").read_text(encoding="utf-8")
        self.assertIn("mg_tabwindow_finalized_cb", main_source)
        self.assertIn("mg_win32_display_filter", main_source)
        self.assertIn("FabulorGtkInternalDragKind kind", main_source)
        self.assertIn("fabulor_pane_clamp_end_size", main_source)
        self.assertIn(
            "MAX (prefs.hex_gui_pane_right_size_min, 1)", main_source
        )
        self.assertIn("fabulor_emoji_picker_viewport_size", main_source)
        trailing_helper = re.search(
            r"fabulor_gtk_horizontal_box_append_trailing\s*"
            r"\([^)]*\)\s*\{(?P<body>.*?)\n\}",
            compat_source,
            re.DOTALL,
        )
        self.assertIsNotNone(trailing_helper)
        self.assertIn(
            "gtk_widget_set_halign (GTK_WIDGET (box), GTK_ALIGN_END)",
            trailing_helper.group("body"),
        )
        self.assertNotIn(
            "gtk_widget_set_hexpand (child, TRUE)", trailing_helper.group("body")
        )
        self.assertNotRegex(
            main_source, r"\bgtk_scrolled_window_new\s*\(\s*(?:NULL|0)"
        )
        self.assertNotRegex(
            main_source,
            r"\bmg_inputbox_icon_release_cb\s*\([^;{]*\bGdkEvent\b",
        )

    def test_menu_sources_complete_frontend_gtk4_specialization(self):
        frontend = ROOT / "src" / "fe-gtk"
        retired_tokens = (
            "GdkEvent",
            "GdkWindow",
            "GtkAccelGroup",
            "GtkCheckMenuItem",
            "GtkMenu",
            "GtkMenuItem",
            "GtkRadioMenuItem",
            "gtk_accel_group_new",
            "gtk_bin_get_child",
            "gtk_box_pack_start",
            "gtk_check_menu_item_",
            "gtk_container_add",
            "gtk_container_get_children",
            "gtk_image_menu_item_new_with_markup",
            "gtk_menu_",
            "gtk_radio_menu_item_",
            "gtk_separator_menu_item_new",
            "gtk_widget_add_accelerator",
            "gtk_widget_destroy",
            "gtk_widget_remove_accelerator",
            "gtk_widget_show_all",
            "usermenu_create",
            "usermenu_destroy",
        )

        for name in GTK4_MENU_SOURCES:
            source = (frontend / name).read_text(encoding="utf-8")
            with self.subTest(source=name):
                self.assertNotIn("GTK_MAJOR_VERSION", source)
                for token in retired_tokens:
                    self.assertNotRegex(source, rf"\b{token}")

        menu_source = (frontend / "menu.c").read_text(encoding="utf-8")
        menu_header = (frontend / "menu.h").read_text(encoding="utf-8")
        self.assertIn("GSimpleActionGroup", menu_source)
        self.assertIn("FabulorContextMenuPresenterGtk4", menu_source)
        self.assertIn("menu_main_composed_model_refresh", menu_source)
        self.assertIn("menu_plugin_context_model", menu_header)

        frontend_sources = list(frontend.rglob("*.c")) + list(frontend.rglob("*.h"))
        for path in frontend_sources:
            with self.subTest(frontend_source=path.relative_to(frontend)):
                self.assertNotIn(
                    "GTK_MAJOR_VERSION", path.read_text(encoding="utf-8")
                )

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
