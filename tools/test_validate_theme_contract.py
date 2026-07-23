#!/usr/bin/env python3

from __future__ import annotations

import pathlib
import subprocess
import tempfile
import unittest

import validate_theme_contract


class ThemeContractValidationTests(unittest.TestCase):
    def setUp(self) -> None:
        fixture_root = pathlib.Path(__file__).resolve().parents[1] / "build" / "theme-contract-tests"
        fixture_root.mkdir(parents=True, exist_ok=True)
        self.temporary = tempfile.TemporaryDirectory(dir=fixture_root)
        self.repo = pathlib.Path(self.temporary.name)
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
            '".hct" "*.hct" "colors.conf" "pevents.conf" '
            "fabulor_theme_archive_read_text_file\n",
        )
        self.write(
            "src/common/theme-archive-reader.c",
            "FABULOR_THEME_ARCHIVE_MAX_BYTES "
            "FABULOR_THEME_ARCHIVE_LIST_MAX_BYTES "
            "FABULOR_THEME_ARCHIVE_TEXT_MAX_BYTES GetSystemDirectoryW "
            "g_subprocess_newv theme_archive_entry_is_safe\n",
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
        self.temporary.cleanup()

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
