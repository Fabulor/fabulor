#!/usr/bin/env python3

from __future__ import annotations

import pathlib
import shutil
import subprocess
import unittest
import uuid

import validate_theme_contract


class ThemeContractValidationTests(unittest.TestCase):
    def setUp(self) -> None:
        fixture_root = (
            pathlib.Path(__file__).resolve().parents[1]
            / "build"
            / "theme-contract-tests-user"
        )
        fixture_root.mkdir(parents=True, exist_ok=True)
        self.repo = fixture_root / uuid.uuid4().hex
        self.repo.mkdir()
        self.write(
            "installer/Components/InstalledMode.wxs",
            """<?xml version="1.0"?>
<Wix><RegistryKey Key="Software\\Classes\\.hct" /></Wix>
""",
        )
        self.write(
            "installer/UX/FabulorBootstrapperApplication.cs",
            'DeleteRegistryTreeIfExists(@"Software\\Classes\\.zct");\n',
        )
        self.write(
            "src/fe-gtk/theme/theme-preferences.c",
            '".hct" "*.hct" "colors.conf" '
            "fabulor_theme_archive_discover "
            "fabulor_theme_archive_read_text_file "
            "theme_palette_transaction_replace gtk_drop_down_new "
            "g_task_run_in_thread\n",
        )
        self.write(
            "src/common/theme-archive-reader.c",
            "FABULOR_THEME_ARCHIVE_MAX_BYTES "
            "FABULOR_THEME_ARCHIVE_LIST_MAX_BYTES "
            "FABULOR_THEME_ARCHIVE_TEXT_MAX_BYTES GetSystemDirectoryW "
            "g_subprocess_newv theme_archive_entry_is_safe "
            "FABULOR_GTK4_ARCHIVE_MAX_BYTES "
            "FABULOR_GTK4_ARCHIVE_MAX_ENTRIES "
            "FABULOR_GTK4_ARCHIVE_MAX_OUTPUT_BYTES "
            "gtk4_archive_copy_bounded gtk4_archive_entry_name_is_safe "
            "gtk4_archive_validate_tree theme_archive_path_is_directory\n",
        )
        self.write(
            "src/fe-gtk/theme/theme-preferences-gtk4.c",
            'fabulor_gtk4_theme_archive_import g_task_run_in_thread '
            "theme_gtk4_controller_reload_catalog "
            "theme_preferences_gtk4_queue_apply g_idle_add_full "
            '"*.tar.xz" "Import theme archive..."\n',
        )
        self.write(
            "src/fe-gtk/theme/theme-gtk4.c",
            "one resolved complete provider\n",
        )
        self.write(
            "src/fe-gtk/theme/theme-runtime.c",
            '"colors.conf" "colors.conf" "colors.conf.new."\n',
        )
        self.write(
            "installer/Components/ShareAssets.wxs",
            """<?xml version="1.0"?>
<Wix><File Source="data\\icons\\fabulor.png" /></Wix>
""",
        )
        subprocess.run(["git", "init", "-q", str(self.repo)], check=True)
        subprocess.run(
            ["git", "-C", str(self.repo), "add", "."], check=True
        )

    def tearDown(self) -> None:
        shutil.rmtree(self.repo, ignore_errors=True)

    def write(self, relative: str, contents: str) -> pathlib.Path:
        path = self.repo / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(contents, encoding="utf-8")
        return path

    def stage(self, relative: str) -> None:
        subprocess.run(
            ["git", "-C", str(self.repo), "add", relative], check=True
        )

    def test_minimal_supported_contract_passes(self) -> None:
        validate_theme_contract.validate(self.repo)

    def test_zct_registration_is_rejected(self) -> None:
        path = self.repo / "installer/Components/InstalledMode.wxs"
        path.write_text(
            """<?xml version="1.0"?>
<Wix>
  <RegistryKey Key="Software\\Classes\\.hct" />
  <RegistryKey Key="Software\\Classes\\.zct" />
</Wix>
""",
            encoding="utf-8",
        )
        with self.assertRaises(validate_theme_contract.ThemeContractError):
            validate_theme_contract.validate(self.repo)

    def test_repository_default_theme_payload_is_rejected(self) -> None:
        self.write("data/themes/fabulor/gtk-4.0/gtk.css", "window {}\n")
        self.stage("data/themes/fabulor/gtk-4.0/gtk.css")
        with self.assertRaises(validate_theme_contract.ThemeContractError):
            validate_theme_contract.validate(self.repo)

    def test_wix_theme_harvest_is_rejected(self) -> None:
        path = self.repo / "installer/Components/ShareAssets.wxs"
        path.write_text(
            """<?xml version="1.0"?>
<Wix><Files Include="$(var.FabulorPayloadRoot)\\share\\themes\\**" /></Wix>
""",
            encoding="utf-8",
        )
        with self.assertRaises(validate_theme_contract.ThemeContractError):
            validate_theme_contract.validate(self.repo)

    def test_archive_reader_path_search_is_rejected(self) -> None:
        path = self.repo / "src/common/theme-archive-reader.c"
        path.write_text(
            path.read_text(encoding="utf-8") + "G_SPAWN_SEARCH_PATH\n",
            encoding="utf-8",
        )
        with self.assertRaises(validate_theme_contract.ThemeContractError):
            validate_theme_contract.validate(self.repo)

    def test_gtk4_archive_import_boundary_is_required(self) -> None:
        path = self.repo / "src/fe-gtk/theme/theme-preferences-gtk4.c"
        path.write_text(
            path.read_text(encoding="utf-8").replace(
                "fabulor_gtk4_theme_archive_import", "removed_archive_import"
            ),
            encoding="utf-8",
        )
        with self.assertRaises(validate_theme_contract.ThemeContractError):
            validate_theme_contract.validate(self.repo)

    def test_layered_gtk4_variant_provider_is_rejected(self) -> None:
        path = self.repo / "src/fe-gtk/theme/theme-gtk4.c"
        path.write_text("variant_provider\n", encoding="utf-8")
        with self.assertRaises(validate_theme_contract.ThemeContractError):
            validate_theme_contract.validate(self.repo)

    def test_legacy_event_import_is_rejected(self) -> None:
        path = self.repo / "src/fe-gtk/theme/theme-preferences.c"
        path.write_text(
            path.read_text(encoding="utf-8") + '"pevents.conf"\n',
            encoding="utf-8",
        )
        with self.assertRaises(validate_theme_contract.ThemeContractError):
            validate_theme_contract.validate(self.repo)

    def test_profile_theme_selector_is_required(self) -> None:
        path = self.repo / "src/fe-gtk/theme/theme-preferences.c"
        path.write_text(
            path.read_text(encoding="utf-8").replace(
                "fabulor_theme_archive_discover", "removed_theme_discovery"
            ),
            encoding="utf-8",
        )
        with self.assertRaises(validate_theme_contract.ThemeContractError):
            validate_theme_contract.validate(self.repo)

    def test_profile_theme_selector_model_double_unref_is_rejected(self) -> None:
        path = self.repo / "src/fe-gtk/theme/theme-preferences.c"
        path.write_text(
            path.read_text(encoding="utf-8") + "g_object_unref (profile_model);\n",
            encoding="utf-8",
        )
        with self.assertRaises(validate_theme_contract.ThemeContractError):
            validate_theme_contract.validate(self.repo)

    def test_retired_gtk3_theme_file_is_rejected(self) -> None:
        self.write("src/common/gtk3-theme-service.c", "retired\n")
        with self.assertRaises(validate_theme_contract.ThemeContractError):
            validate_theme_contract.validate(self.repo)

    def test_retired_gtk3_theme_reference_is_rejected(self) -> None:
        path = self.repo / "src/fe-gtk/theme/theme-preferences.c"
        path.write_text(
            path.read_text(encoding="utf-8") + "theme_gtk3_apply_current\n",
            encoding="utf-8",
        )
        with self.assertRaises(validate_theme_contract.ThemeContractError):
            validate_theme_contract.validate(self.repo)


if __name__ == "__main__":
    unittest.main()
