import pathlib
import json
import re
import unittest
import xml.etree.ElementTree as ET


ROOT = pathlib.Path(__file__).resolve().parents[2]
INSTALLER = ROOT / "installer"
PROPS = ROOT / "win32" / "fabulor.props"
SOLUTION = ROOT / "win32" / "fabulor.sln"
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
IRC_PROTOCOL_SOURCE = ROOT / "src" / "common" / "proto-irc.c"
GTK4_CHANNEL_LIST_SOURCE = ROOT / "src" / "fe-gtk" / "channel-list.c"
GTK4_MENU_SOURCE = ROOT / "src" / "fe-gtk" / "menu.c"
PLUGIN_LOADER_SOURCE = ROOT / "src" / "common" / "plugin.c"
GTK4_TRAY_SOURCES = (
    "plugin-tray.c",
    "tray-menu-presenter-gtk4.c",
)
LEGACY_BUILD_GRAPH = (
    ROOT / "Makefile",
    ROOT / "meson_post_install.py",
    ROOT / "src" / "meson.build",
    ROOT / "src" / "common" / "meson.build",
    ROOT / "src" / "common" / "dbus" / "meson.build",
    ROOT / "src" / "fe-gtk" / "meson.build",
    ROOT / "plugins" / "meson.build",
    ROOT / "plugins" / "checksum" / "meson.build",
    ROOT / "plugins" / "exec" / "meson.build",
    ROOT / "plugins" / "fishlim" / "meson.build",
    ROOT / "plugins" / "fishlim" / "tests" / "meson.build",
    ROOT / "plugins" / "lua" / "meson.build",
    ROOT / "plugins" / "perl" / "meson.build",
    ROOT / "plugins" / "python" / "meson.build",
    ROOT / "plugins" / "sysinfo" / "meson.build",
    ROOT / "plugins" / "upd" / "meson.build",
    ROOT / "data" / "meson.build",
    ROOT / "data" / "icons" / "meson.build",
    ROOT / "data" / "man" / "meson.build",
    ROOT / "data" / "misc" / "meson.build",
    ROOT / "data" / "pkgconfig" / "meson.build",
    ROOT / "po" / "meson.build",
)
GTK4_PROBE_MESON = ROOT / "tools" / "gtk4" / "meson.build"
GTK4_APPLICATION_SOURCE = ROOT / "src" / "fe-gtk" / "fe-gtk.c"
GTK4_SERVER_LIST_SOURCE = ROOT / "src" / "fe-gtk" / "servlistgui.c"
COMMON_SERVER_LIST_SOURCE = ROOT / "src" / "common" / "servlist.c"
COMMON_HISTORY_SOURCE = ROOT / "src" / "common" / "history.c"
COMMON_HISTORY_HEADER = ROOT / "src" / "common" / "history.h"
COMMON_OUTBOUND_SOURCE = ROOT / "src" / "common" / "outbound.c"
COMMON_APPLICATION_SOURCE = ROOT / "src" / "common" / "fabulor.c"
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
    def test_inherited_updater_is_retired(self):
        self.assertFalse((ROOT / "plugins" / "upd" / "upd.c").exists())
        self.assertFalse((ROOT / "plugins" / "upd" / "upd.vcxproj").exists())

        sources = (
            SOLUTION,
            PROPS,
            NATIVE_EXTENSIONS,
            PLUGIN_LOADER_SOURCE,
            INSTALLER / "Fabulor.wixproj",
            INSTALLER / "ProductGtk4.wxs",
            INSTALLER / "Components" / "PluginsGtk4.wxs",
            INSTALLER / "UX" / "FabulorBootstrapperApplication.cs",
            INSTALLER / "UX" / "InstallerFeatureSelection.cs",
            INSTALLER / "UX" / "MainWindow.xaml",
            ROOT / "tools" / "gtk4" / "native-extension-contract.json",
            ROOT / "tools" / "gtk4" / "production-support-contract.json",
            ROOT / "tools" / "gtk4" / "stage_production_support.py",
            ROOT / ".github" / "workflows" / "tests.yml",
            ROOT / ".github" / "workflows" / "windows-build.yml",
        )
        retired_tokens = (
            "zoitechat.org/appcast.xml",
            "WinSparkle",
            "hcupd",
            "IncludeUpdatePlugin",
            "UpdatePluginFeature",
        )

        for path in sources:
            source = path.read_text(encoding="utf-8")
            for token in retired_tokens:
                self.assertNotIn(token, source, f"{token} remains in {path}")

    def test_future_updater_remains_behind_signed_feed_gate(self):
        design = (
            ROOT / "docs" / "security" / "signed-update-feed-design.md"
        ).read_text(encoding="utf-8")
        roadmap = (ROOT / "To-Do.md").read_text(encoding="utf-8")

        required_design = (
            "Status: design accepted; implementation and activation remain blocked.",
            "The Update Framework (TUF) specification version 1.0.33",
            "WinVerifyTrust",
            "2 of 3",
            "rollback",
            "SubjectPublicKeyInfo",
            "The in-client action remains absent until all of these are true:",
        )
        for requirement in required_design:
            self.assertIn(requirement, design)

        self.assertIn(
            "- [ ] Provision the update signing identities, metadata origin,",
            roadmap,
        )

    def test_list_numeric_accepts_an_empty_trailing_topic(self):
        source = IRC_PROTOCOL_SOURCE.read_text(encoding="utf-8")
        list_numeric = re.search(
            r"case 322:(.*?)\n\tcase 323:", source, re.DOTALL
        )

        self.assertIsNotNone(list_numeric)
        self.assertIn(
            "const char *topic = irc_trailing_parameter_text (word_eol[6]);",
            list_numeric.group(1),
        )
        self.assertIn(
            "return parameter[0] == ':' ? parameter + 1 : parameter;",
            source,
        )
        self.assertNotIn("word_eol[6] + 1", list_numeric.group(1))

    def test_channel_list_sorting_is_null_safe(self):
        source = GTK4_CHANNEL_LIST_SOURCE.read_text(encoding="utf-8")

        self.assertIn("gtk_custom_sorter_new (channel_row_compare", source)
        self.assertIn("g_strcmp0 (row1->topic, row2->topic)", source)
        self.assertIn(
            "g_strcmp0 (row1->collation_key, row2->collation_key)", source
        )
        self.assertNotIn("gtk_string_sorter_new", source)

    def test_server_menu_channel_list_requests_a_download(self):
        source = GTK4_MENU_SOURCE.read_text(encoding="utf-8")
        callback = re.search(
            r"menu_chanlist \(.*?\n\}", source, re.DOTALL
        )

        self.assertIsNotNone(callback)
        self.assertIn(
            "chanlist_opengui (current_sess->server, TRUE);",
            callback.group(0),
        )

    def test_bootstrapper_application_is_self_contained(self):
        project = ET.parse(INSTALLER / "UX" / "Fabulor.BA.csproj").getroot()
        properties = {
            child.tag: (child.text or "").strip()
            for group in project.findall("PropertyGroup")
            for child in group
        }

        self.assertEqual(properties.get("RuntimeIdentifier"), "win-x64")
        self.assertEqual(properties.get("SelfContained"), "true")
        self.assertEqual(properties.get("PublishSingleFile"), "true")
        self.assertEqual(
            properties.get("IncludeNativeLibrariesForSelfExtract"), "true"
        )
        self.assertEqual(properties.get("EnableCompressionInSingleFile"), "true")
        self.assertEqual(properties.get("PublishTrimmed"), "false")

        setup_project = (
            INSTALLER / "Bootstrapper" / "FabulorSetup.wixproj"
        ).read_text(encoding="utf-8")
        self.assertIn(r"net8.0-windows\win-x64\publish", setup_project)
        self.assertIn('Targets="Restore;Publish"', setup_project)

        bundle = ET.parse(INSTALLER / "Bootstrapper" / "Bundle.wxs").getroot()
        application = bundle.find(".//w:BootstrapperApplication", WIX_NS)
        self.assertIsNotNone(application)
        self.assertEqual(
            application.get("SourceFile"),
            r"$(var.BundleBootstrapperApplicationRoot)\Fabulor.BA.exe",
        )
        self.assertEqual(application.findall("w:Payload", WIX_NS), [])

    def test_installer_ux_keeps_diagnostics_and_shortcuts_owned(self):
        product = (INSTALLER / "ProductGtk4.wxs").read_text(encoding="utf-8")
        components = (
            INSTALLER / "Components" / "InstalledMode.wxs"
        ).read_text(encoding="utf-8")
        components_root = ET.parse(
            INSTALLER / "Components" / "InstalledMode.wxs"
        ).getroot()
        feature_selection = (
            INSTALLER / "UX" / "InstallerFeatureSelection.cs"
        ).read_text(encoding="utf-8")
        bootstrapper = (
            INSTALLER / "UX" / "FabulorBootstrapperApplication.cs"
        ).read_text(encoding="utf-8")
        window = (INSTALLER / "UX" / "MainWindow.xaml").read_text(
            encoding="utf-8"
        )
        session_log = (INSTALLER / "UX" / "InstallerSessionLog.cs").read_text(
            encoding="utf-8"
        )

        self.assertIn('Feature Id="DesktopShortcutFeature"', product)
        self.assertIn('ComponentGroupRef Id="DesktopShortcutComponents"', product)
        self.assertIn('ComponentGroup Id="DesktopShortcutComponents"', components)
        self.assertIn('Directory="DesktopFolder"', components)
        self.assertIn("IncludeDesktopShortcut", feature_selection)
        self.assertIn("DesktopShortcutFeatureId", bootstrapper)
        self.assertIn(
            'RegistryValueExists(Registry.CurrentUser, @"Software\\Fabulor\\Installer", "DesktopShortcut")',
            bootstrapper,
        )
        self.assertIn('x:Name="DesktopShortcutCheckBox"', window)
        self.assertIn('Value="Select or clear this option."', window)
        self.assertIn('x:Name="AdvancedOptionsExpander"', window)
        self.assertIn('x:Name="DetailsExpander"', window)
        self.assertIn('x:Name="LaunchButton"', window)
        self.assertIn('SpecialFolder.LocalApplicationData', session_log)
        self.assertIn("Path.IsPathFullyQualified(localApplicationDataPath)", session_log)
        self.assertIn('@"Fabulor\\Installer\\Logs"', session_log)
        self.assertIn("SuccessfulLogRetentionCount = 10", session_log)

        window_code = (INSTALLER / "UX" / "MainWindow.xaml.cs").read_text(
            encoding="utf-8"
        )
        portable_policy = (
            INSTALLER / "UX" / "PortableInstallLocationPolicy.cs"
        ).read_text(encoding="utf-8")
        self.assertIn("catch (COMException ex)", window_code)
        self.assertIn("showingInstallFolderWarning", window_code)
        self.assertIn(
            "PortableInstallLocationPolicy.IsProtectedLocation", bootstrapper
        )
        self.assertIn(
            "PortableInstallLocationPolicy.GetDefaultInstallFolder", bootstrapper
        )
        self.assertIn("Environment.SpecialFolder.ProgramFiles", portable_policy)
        self.assertIn("Environment.SpecialFolder.ProgramFilesX86", portable_policy)
        self.assertIn("Environment.SpecialFolder.Windows", portable_policy)
        self.assertIn('Path.Combine(userProfile, "Fabulor Portable")', portable_policy)

        self.assertIn('Key="Software\\Classes\\irc"', components)
        self.assertIn('Key="Software\\Classes\\ircs"', components)
        for component_id, handler_property in (
            ("IrcProtocolFallbackRegistration", "IRC_PROTOCOL_HANDLER"),
            ("IrcsProtocolFallbackRegistration", "IRCS_PROTOCOL_HANDLER"),
        ):
            component = components_root.find(
                f".//w:Component[@Id='{component_id}']", WIX_NS
            )
            self.assertIsNotNone(component)
            self.assertEqual(component.get("NeverOverwrite"), "yes")
            self.assertIn(f"NOT {handler_property}", component.get("Condition"))

            handler_search = components_root.find(
                f".//w:Property[@Id='{handler_property}']/w:RegistrySearch",
                WIX_NS,
            )
            self.assertIsNotNone(handler_search)
            self.assertEqual(handler_search.get("Type"), "raw")
            self.assertEqual(handler_search.get("Bitness"), "always64")
        self.assertEqual(
            components.count(
                'Name="URL Protocol" Type="string" Value="" KeyPath="yes"'
            ),
            2,
        )
        self.assertIn('Key="Software\\Classes\\Fabulor.Url.Irc"', components)
        self.assertIn(
            'Key="Software\\Classes\\Fabulor.Url.IrcSecure"', components
        )
        self.assertIn('Key="Software\\RegisteredApplications"', components)
        self.assertIn('Value="Software\\Fabulor\\Capabilities"', components)
        self.assertIn('Key="Software\\Fabulor\\Capabilities\\UrlAssociations"', components)
        self.assertIn('Name="irc" Type="string" Value="Fabulor.Url.Irc"', components)
        self.assertIn(
            'Name="ircs" Type="string" Value="Fabulor.Url.IrcSecure"',
            components,
        )
        self.assertEqual(components.count('--url=&quot;%1&quot;'), 4)
        self.assertIn("NotifyShellAssociationsChanged();", bootstrapper)
        self.assertIn("DeleteOwnedIrcProtocolSchemeClaims", bootstrapper)
        self.assertEqual(
            bootstrapper.count("this.DeleteOwnedIrcProtocolSchemeClaims();"),
            1,
        )
        self.assertIn('this.DeleteOwnedIrcProtocolSchemeClaim("irc");', bootstrapper)
        self.assertIn('this.DeleteOwnedIrcProtocolSchemeClaim("ircs");', bootstrapper)
        self.assertIn("RegexOptions.IgnoreCase | RegexOptions.CultureInvariant", bootstrapper)
        self.assertIn(r'--url=""%1""', bootstrapper)

    def test_production_product_keeps_upgrade_identity(self):
        root = ET.parse(INSTALLER / "ProductGtk4.wxs").getroot()
        package = root.find("w:Package", WIX_NS)

        self.assertIsNotNone(package)
        self.assertEqual(package.get("Name"), "$(var.FabulorProductName)")
        self.assertEqual(
            package.get("UpgradeCode"),
            "8F6C0C7E-9A4D-4E4C-9F8C-2B6F5A4E9C11",
        )
        install = root.find(".//w:Directory[@Id='INSTALLFOLDER']", WIX_NS)
        self.assertEqual(install.get("Name"), "Fabulor")

        major_upgrade = root.find("w:Package/w:MajorUpgrade", WIX_NS)
        self.assertIsNotNone(major_upgrade)
        self.assertEqual(major_upgrade.get("Schedule"), "afterInstallInitialize")

        bootstrapper = (
            INSTALLER / "UX" / "FabulorBootstrapperApplication.cs"
        ).read_text(encoding="utf-8")
        self.assertIn(
            "this.pendingAction == LaunchAction.Install && this.isFabulorMsiInstalled",
            bootstrapper,
        )
        self.assertIn("e.State = RequestState.None;", bootstrapper)
        self.assertIn("this.detectedRelatedBundleCachePaths.Add(bundleCachePath);", bootstrapper)
        self.assertIn("this.RemoveStaleBundleDependencyDependents(preservedBundleCode);", bootstrapper)
        self.assertIn("this.RemoveStaleMsiDependencyRegistrations();", bootstrapper)
        self.assertIn("this.RemoveDetectedRelatedBundleCaches(preservedBundlePath);", bootstrapper)
        self.assertIn("!Guid.TryParse(cacheDirectoryName, out _)", bootstrapper)

    def test_python_runtime_payload_keeps_loader_dlls(self):
        project = (INSTALLER / "Fabulor.wixproj").read_text(encoding="utf-8")
        components = (
            INSTALLER / "Components" / "PluginRuntimesGtk4.wxs"
        ).read_text(encoding="utf-8")

        self.assertIn(r"Runtime\Python314\python314.dll", project)
        self.assertIn(r"Runtime\Python314\python3.dll", project)
        self.assertIn(
            r'Files Include="$(var.Gtk4PluginHostRoot)\Runtime\Python314\**"',
            components,
        )

    def test_about_help_and_licence_contract(self):
        menu = (ROOT / "src" / "fe-gtk" / "menu.c").read_text(encoding="utf-8")
        pixmaps = (ROOT / "src" / "fe-gtk" / "pixmaps.c").read_text(
            encoding="utf-8"
        )
        resources = (ROOT / "data" / "fabulor.gresource.xml").read_text(
            encoding="utf-8"
        )
        share = (INSTALLER / "Components" / "ShareGtk4.wxs").read_text(
            encoding="utf-8"
        )

        self.assertTrue((ROOT / "data" / "icons" / "fabulor-about.png").is_file())
        self.assertIn("fabulor-about.png", resources)
        self.assertIn(
            'gdk_pixbuf_new_from_resource (\n'
            '\t\t"/icons/fabulor-about.png", NULL);',
            pixmaps,
        )
        self.assertIn("GTK_LICENSE_GPL_3_0_ONLY", menu)
        self.assertNotIn("GTK_LICENSE_GPL_2_0_ONLY", menu)
        for action in ("contents", "project-website", "report-issue", "about"):
            self.assertIn(f'"{action}"', menu)
        self.assertIn(r"..\..\Licence.md", share)
        self.assertIn('Name="Licence.md"', share)
        self.assertNotIn(r"..\..\COPYING", share)

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

    def test_repository_cleanup_stage4_has_one_windows_backend_set(self):
        removed_paths = (
            ROOT / "src" / "fe-text" / "fe-text.vcxproj",
            ROOT / "src" / "fe-text" / "fe-text.c",
            ROOT / "src" / "dirent" / "dirent-win32.h",
            ROOT / "src" / "fe-gtk" / "notifications" / "notification-dummy.c",
            ROOT / "src" / "fe-gtk" / "notifications" / "notification-freedesktop.c",
            ROOT / "plugins" / "sysinfo" / "osx" / "backend.m",
            ROOT / "plugins" / "sysinfo" / "shared" / "df.c",
            ROOT / "plugins" / "sysinfo" / "shared" / "df.h",
        )
        for path in removed_paths:
            self.assertFalse(path.exists(), f"retired Stage 4 path remains: {path}")

        frontend_project = (ROOT / "src" / "fe-gtk" / "fe-gtk.vcxproj").read_text(
            encoding="utf-8"
        )
        common_project = (ROOT / "src" / "common" / "common.vcxproj").read_text(
            encoding="utf-8"
        )
        sysinfo_project = (
            ROOT / "plugins" / "sysinfo" / "sysinfo.vcxproj"
        ).read_text(encoding="utf-8")
        enchant_project = (
            ROOT / "tools" / "enchant-msvc" / "CMakeLists.txt"
        ).read_text(encoding="utf-8")

        self.assertIn(r"notifications\notification-windows.c", frontend_project)
        self.assertIn(r"sysinfo\win32\backend.c", common_project)
        self.assertIn(r"win32\backend.c", sysinfo_project)
        self.assertIn("compat/flock.c", enchant_project)
        self.assertIn("compat/relocatable.c", enchant_project)
        self.assertTrue(GTK_COMPAT.is_file())

    def test_repository_cleanup_stage5_separates_current_and_archived_docs(self):
        retired_live_paths = (
            ROOT / "docs" / "gtk4" / "migration-plan.md",
            ROOT / "docs" / "gtk4" / "api-inventory.md",
            ROOT / "docs" / "gtk4" / "validation-log.md",
            ROOT / "docs" / "security" / "manifest-plugin-disabled-state-audit.md",
            ROOT / "docs" / "security" / "enchant-windows-crash-analysis.md",
        )
        for path in retired_live_paths:
            self.assertFalse(path.exists(), f"historical document is not archived: {path}")

        archived_paths = (
            ROOT / "docs" / "gtk4" / "archive" / "README.md",
            ROOT / "docs" / "gtk4" / "archive" / "migration-plan.md",
            ROOT / "docs" / "gtk4" / "archive" / "api-inventory.md",
            ROOT / "docs" / "gtk4" / "archive" / "validation-log.md",
            ROOT / "docs" / "security" / "archive" / "README.md",
            ROOT
            / "docs"
            / "security"
            / "archive"
            / "manifest-plugin-disabled-state-audit.md",
            ROOT
            / "docs"
            / "security"
            / "archive"
            / "enchant-windows-crash-analysis.md",
        )
        for path in archived_paths:
            self.assertTrue(path.is_file(), f"archived evidence is missing: {path}")

        current_paths = (
            ROOT / "docs" / "gtk4" / "runtime-packaging.md",
            ROOT / "docs" / "gtk4" / "theme-architecture.md",
            ROOT / "docs" / "plugins" / "plugin-schema-and-troubleshooting.md",
            ROOT / "docs" / "security" / "README.md",
            ROOT / "docs" / "security" / "signed-update-feed-design.md",
            ROOT / "docs" / "security" / "trusted-config.md",
        )
        for path in current_paths:
            self.assertTrue(path.is_file(), f"current guidance is missing: {path}")

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
        self.assertIn("ManagedPluginHostBuildRoot", source)
        self.assertIn("ManagedPluginHostOutput", source)
        self.assertIn("DestinationFolder=\"$(Gtk4PluginHostRoot)\\Runtime\\DotNet\"", source)
        managed_components = (
            INSTALLER / "Components" / "PluginRuntimesGtk4.wxs"
        ).read_text(encoding="utf-8")
        self.assertIn("$(var.ManagedPluginHostBuildRoot)", managed_components)

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

    def test_msvc_build_names_are_fabulor_only(self):
        props = PROPS.read_text(encoding="utf-8")
        solution = SOLUTION.read_text(encoding="utf-8-sig")

        self.assertFalse((ROOT / "win32" / "zoitechat.props").exists())
        self.assertFalse((ROOT / "win32" / "zoitechat.sln").exists())
        self.assertIn('= "fabulor", "fabulor"', solution)
        for token in (
            "ZoiteChatBuild",
            "ZoiteChatPlatform",
            "ZoiteChatBin",
            "ZoiteChatObj",
            "ZoiteChatLib",
            "ZoiteChatPdb",
            "ZoiteChatRel",
        ):
            self.assertNotIn(token, props)

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
        self.assertIn("tray_bind_current_context", plugin_source)
        self.assertRegex(
            plugin_source,
            r"if\s*\(\s*!tray_bind_current_context\s*\(\s*\)\s*\)",
        )

        core_plugin = (ROOT / "src" / "common" / "plugin.c").read_text(
            encoding="utf-8"
        )
        core_lifecycle = (ROOT / "src" / "common" / "fabulor.c").read_text(
            encoding="utf-8"
        )
        self.assertIn("plugin_rebind_context", core_plugin)
        self.assertRegex(
            core_plugin,
            r"if\s*\(\s*is_session\s*\(\s*ph->context\s*\)\s*\)",
        )
        self.assertRegex(
            core_lifecycle,
            r"plugin_rebind_context\s*\(\s*killsess\s*,\s*current_sess\s*\)\s*;",
        )

        for path in LEGACY_BUILD_GRAPH:
            with self.subTest(path=path.relative_to(ROOT)):
                self.assertFalse(path.exists())
        self.assertTrue(GTK4_PROBE_MESON.is_file())

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

    def test_server_credentials_and_client_certificates_are_self_contained(self):
        frontend = GTK4_SERVER_LIST_SOURCE.read_text(encoding="utf-8")
        common = COMMON_SERVER_LIST_SOURCE.read_text(encoding="utf-8")

        for retired_label in (
            "Encrypt saved password",
            "Move password to keyring",
            "Generate client SSL cert",
        ):
            self.assertNotIn(retired_label, frontend)
        self.assertNotIn("G_SPAWN_SEARCH_PATH", frontend)
        self.assertNotRegex(frontend, r'argv\[\d+\]\s*=\s*"openssl"')

        for retained_label in (
            "Store password in Windows Credential Manager",
            "Import client certificate...",
            "Certificate details",
            "Remove certificate",
        ):
            self.assertIn(retained_label, frontend)
        self.assertIn("servlist_open_client_cert_context", frontend)
        self.assertIn("SSL_CTX_check_private_key", frontend)

        self.assertIn(
            "!portable_mode () && secretstore_is_keyring_available ()",
            common,
        )
        self.assertIn("servlist_password_is_encrypted (net->pass)", common)
        self.assertIn(
            "encrypted = servlist_password_encrypt_for_storage (net->pass);",
            common,
        )

    def test_saved_input_history_is_network_and_channel_scoped(self):
        history = COMMON_HISTORY_SOURCE.read_text(encoding="utf-8")
        header = COMMON_HISTORY_HEADER.read_text(encoding="utf-8")
        outbound = COMMON_OUTBOUND_SOURCE.read_text(encoding="utf-8")
        application = COMMON_APPLICATION_SOURCE.read_text(encoding="utf-8")

        self.assertNotIn("input-history.conf", history)
        self.assertNotIn("shared_history", history)
        self.assertIn('#define HISTORY_DIRECTORY "history"', history)
        self.assertIn('#define HISTORY_EXTENSION ".log"', history)
        self.assertIn('target = "server";', history)
        self.assertIn("server_get_network (sess->server, FALSE)", history)
        self.assertIn("history_filename_component (network)", history)
        self.assertIn("history_filename_component (target)", history)
        self.assertIn("g_mkdir_with_parents", history)

        self.assertIn("struct session *owner;", header)
        self.assertIn("char *storage_path;", header)
        self.assertIn("history_restore (&sess->history, sess);", application)
        self.assertIn("for (list = sess_list; list; list = list->next)", history)
        self.assertIn("history_erase (&sess->history);", outbound)
        self.assertIn('g_ascii_strcasecmp (reason, "LOG")', outbound)
        self.assertIn("log_clear (sess)", outbound)
        self.assertIn(
            "CLEAR [ALL|HISTORY|LOG|[-]<amount>]",
            outbound,
        )

    def test_channel_and_ban_dialogs_are_gtk4_only(self):
        frontend = ROOT / "src" / "fe-gtk"
        retired_tokens = (
            "chanlist_copychannel",
            "chanlist_copytopic",
            "chanlist_icon_menu_item",
            "chanlist_menu_destroy",
            "gtk_box_pack_start",
            "gtk_container_add",
            "gtk_menu_(?!button_)",
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
            "gtk_menu_(?!button_)",
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
            "MAX (MG_USERLIST_MIN_WIDTH,\n"
            "                    prefs.hex_gui_pane_right_size_min)",
            main_source,
        )
        self.assertIn("fabulor_emoji_picker_viewport_size", main_source)
        self.assertIn("gtk_drop_down_new", main_source)
        self.assertIn("mg_emoji_category_changed_cb", main_source)
        self.assertIn("mg_emoji_popover_close_cb", main_source)
        self.assertIn('"window-close-symbolic"', main_source)
        self.assertIn("gtk_flow_box_append", main_source)
        self.assertIn(
            "GTK_POLICY_NEVER,\n"
            "                                        GTK_POLICY_AUTOMATIC",
            main_source,
        )
        self.assertNotIn("mg_emoji_grid_scroller_new", main_source)
        self.assertNotIn("gtk_stack_switcher_new", main_source)
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

    def test_legacy_copy_payload_namespace_is_retired(self):
        self.assertFalse((ROOT / "win32" / "copy").exists())

        required_assets = (
            "data/windows/readme.url",
            "data/icons/adwaita/ATTRIBUTION.txt",
            "data/icons/gtkpref.png",
            "data/icons/music.png",
            "data/icons/system.png",
            "data/iso-codes/iso_3166.xml",
            "data/iso-codes/iso_639.xml",
        )
        for relative in required_assets:
            with self.subTest(asset=relative):
                self.assertTrue((ROOT / relative).is_file())

        installer_sources = (
            INSTALLER / "Components" / "CoreGtk4.wxs",
            INSTALLER / "Components" / "ShareAssets.wxs",
        )
        for source_path in installer_sources:
            source = source_path.read_text(encoding="utf-8")
            self.assertNotIn("win32\\copy", source)
            self.assertNotIn("win32/copy", source)

        self.assertFalse((ROOT / "data" / "icons" / "download.png").exists())

    def test_perl_integration_is_retired(self):
        self.assertFalse((ROOT / "plugins" / "perl").exists())

        config_template = (ROOT / "win32" / "config.h.tt").read_text(
            encoding="utf-8"
        )
        cfgfiles = (ROOT / "src" / "common" / "cfgfiles.c").read_text(
            encoding="utf-8"
        )
        preferences = (ROOT / "src" / "common" / "fabulor.h").read_text(
            encoding="utf-8"
        )
        outbound = (ROOT / "src" / "common" / "outbound.c").read_text(
            encoding="utf-8"
        )

        self.assertNotIn("OLD_PERL", config_template)
        self.assertNotIn("perl_warnings", cfgfiles)
        self.assertNotIn("hex_perl_warnings", preferences)
        self.assertNotIn("install the Perl or Python plugin", outbound)

    def test_python312_repository_residue_is_retired(self):
        self.assertFalse((ROOT / "Runtime" / "Python312").exists())
        self.assertFalse(
            (INSTALLER / "Components" / "Python312.wxs.bak").exists()
        )
        self.assertFalse((INSTALLER / "wix-build.binlog").exists())

    def test_windows_support_dependencies_are_pinned(self):
        manifest = json.loads(VCPKG_MANIFEST.read_text(encoding="utf-8"))
        configuration = json.loads(VCPKG_CONFIGURATION.read_text(encoding="utf-8"))

        self.assertEqual(manifest["dependencies"], ["openssl"])
        self.assertRegex(
            configuration["default-registry"]["baseline"], r"^[0-9a-f]{40}$"
        )


if __name__ == "__main__":
    unittest.main()
