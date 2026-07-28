#!/usr/bin/env python3
"""Validate Fabulor's supported theme formats and packaging boundary."""

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys
import xml.etree.ElementTree as ET


class ThemeContractError(RuntimeError):
    pass


def validate_gtk3_theme_retirement(repo: pathlib.Path) -> None:
    retired_paths = (
        repo / "src" / "common" / "gtk3-theme-service.c",
        repo / "src" / "common" / "gtk3-theme-service.h",
        repo / "src" / "common" / "tests" / "test-gtk3-theme-service.c",
        repo / "src" / "fe-gtk" / "theme" / "theme-gtk3.c",
        repo / "src" / "fe-gtk" / "theme" / "theme-gtk3.h",
        repo / "src" / "fe-gtk" / "theme" / "tests" / "test-theme-gtk3-stub.c",
        repo / "src" / "fe-gtk" / "theme" / "tests" / "test-theme-gtk3-settings.c",
        repo / "src" / "fe-gtk" / "theme" / "tests" / "test-theme-preferences-gtk3-populate.c",
    )
    present_paths = [str(path.relative_to(repo)) for path in retired_paths if path.exists()]
    if present_paths:
        raise ThemeContractError(
            "Retired GTK3 theme files are present: " + ", ".join(present_paths)
        )

    forbidden_tokens = (
        "gtk3-theme-service",
        "theme-gtk3",
        "theme_gtk3",
        "hex_gui_gtk3_",
        "gui_gtk3_",
    )
    source_roots = (repo / "src", repo / "tools" / "gtk4")
    allowed_suffixes = {".c", ".h", ".build", ".vcxproj", ".filters"}
    violations = []
    for root in source_roots:
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if not path.is_file() or path.suffix.lower() not in allowed_suffixes:
                continue
            contents = path.read_text(encoding="utf-8")
            for token in forbidden_tokens:
                if token in contents:
                    violations.append(f"{path.relative_to(repo)}: {token}")
    if violations:
        raise ThemeContractError(
            "Retired GTK3 theme references are present: " + ", ".join(violations)
        )


def read_text(path: pathlib.Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except OSError as exc:
        raise ThemeContractError(f"Unable to read {path}: {exc}") from exc


def tracked_files(repo: pathlib.Path) -> list[pathlib.PurePosixPath]:
    result = subprocess.run(
        ["git", "-C", str(repo), "ls-files", "-z"],
        check=False,
        capture_output=True,
    )
    if result.returncode != 0:
        raise ThemeContractError(
            f"Unable to enumerate tracked files: {result.stderr.decode(errors='replace')}"
        )
    return [
        pathlib.PurePosixPath(item.decode("utf-8"))
        for item in result.stdout.split(b"\0")
        if item
    ]


def wix_registry_extensions(path: pathlib.Path) -> set[str]:
    try:
        root = ET.parse(path).getroot()
    except (OSError, ET.ParseError) as exc:
        raise ThemeContractError(f"Unable to parse {path}: {exc}") from exc
    extensions: set[str] = set()
    prefix = "software\\classes\\."
    for element in root.iter():
        key = element.attrib.get("Key", "").casefold()
        if key.startswith(prefix) and "\\" not in key[len(prefix) :]:
            extensions.add(key.removeprefix("software\\classes\\"))
    return extensions


def validate_associations(repo: pathlib.Path) -> None:
    installed_mode = repo / "installer" / "Components" / "InstalledMode.wxs"
    extensions = wix_registry_extensions(installed_mode)
    if extensions != {".hct"}:
        raise ThemeContractError(
            f"Active WiX theme associations must be exactly .hct; found {sorted(extensions)}"
        )

    cleanup = read_text(
        repo / "installer" / "UX" / "FabulorBootstrapperApplication.cs"
    ).casefold()
    if 'software\\classes\\.zct' not in cleanup:
        raise ThemeContractError("Installer upgrade cleanup must retain stale .zct removal.")


def validate_import_contract(repo: pathlib.Path) -> None:
    preferences = read_text(
        repo / "src" / "fe-gtk" / "theme" / "theme-preferences.c"
    ).casefold()
    required = (
        '".hct"',
        '"colors.conf"',
        '"*.hct"',
        "fabulor_theme_archive_discover",
        "fabulor_theme_archive_read_text_file",
        "theme_palette_transaction_replace",
        "gtk_drop_down_new",
        "g_task_run_in_thread",
    )
    missing = [token for token in required if token not in preferences]
    if missing:
        raise ThemeContractError(
            f"Theme preferences are missing supported import tokens: {', '.join(missing)}"
        )
    if '"pevents.conf"' in preferences:
        raise ThemeContractError(
            "The active palette importer must not install legacy pevents.conf data."
        )
    if '".zct"' in preferences or '"*.zct"' in preferences:
        raise ThemeContractError("The active theme importer must not accept retired .zct files.")
    if "zoitechat_gtk3_theme_service_read_archive_text_file" in preferences:
        raise ThemeContractError("The active .hct importer must not depend on the GTK3 theme service.")
    if "g_object_unref (profile_model)" in preferences:
        raise ThemeContractError(
            "GtkDropDown owns the profile model passed to gtk_drop_down_new."
        )

    archive_reader = read_text(
        repo / "src" / "common" / "theme-archive-reader.c"
    )
    required_reader_tokens = (
        "FABULOR_THEME_ARCHIVE_MAX_BYTES",
        "FABULOR_THEME_ARCHIVE_LIST_MAX_BYTES",
        "FABULOR_THEME_ARCHIVE_TEXT_MAX_BYTES",
        "GetSystemDirectoryW",
        "g_subprocess_newv",
        "theme_archive_entry_is_safe",
        "FABULOR_GTK4_ARCHIVE_MAX_BYTES",
        "FABULOR_GTK4_ARCHIVE_MAX_ENTRIES",
        "FABULOR_GTK4_ARCHIVE_MAX_OUTPUT_BYTES",
        "gtk4_archive_copy_bounded",
        "gtk4_archive_entry_name_is_safe",
        "gtk4_archive_validate_tree",
        "theme_archive_path_is_directory",
    )
    missing_reader = [
        token for token in required_reader_tokens if token not in archive_reader
    ]
    if missing_reader:
        raise ThemeContractError(
            "The .hct archive reader is missing containment tokens: "
            + ", ".join(missing_reader)
        )
    forbidden_reader_tokens = ("G_SPAWN_SEARCH_PATH", "g_spawn_command_line")
    present_forbidden = [
        token for token in forbidden_reader_tokens if token in archive_reader
    ]
    if present_forbidden:
        raise ThemeContractError(
            "The .hct archive reader uses unsafe process discovery/invocation: "
            + ", ".join(present_forbidden)
        )

    gtk4_preferences = read_text(
        repo / "src" / "fe-gtk" / "theme" / "theme-preferences-gtk4.c"
    )
    required_gtk4_import_tokens = (
        "fabulor_gtk4_theme_archive_import",
        "g_task_run_in_thread",
        "theme_gtk4_controller_reload_catalog",
        "theme_preferences_gtk4_queue_apply",
        "g_idle_add_full",
        '"*.tar.xz"',
        "Import theme archive...",
    )
    missing_gtk4_import = [
        token for token in required_gtk4_import_tokens
        if token not in gtk4_preferences
    ]
    if missing_gtk4_import:
        raise ThemeContractError(
            "GTK4 preferences are missing contained archive import tokens: "
            + ", ".join(missing_gtk4_import)
        )

    gtk4_adapter = read_text(
        repo / "src" / "fe-gtk" / "theme" / "theme-gtk4.c"
    )
    if "variant_provider" in gtk4_adapter:
        raise ThemeContractError(
            "GTK4 themes must install one resolved light or dark provider, not layered full providers."
        )

    runtime = read_text(
        repo / "src" / "fe-gtk" / "theme" / "theme-runtime.c"
    ).casefold()
    if runtime.count('"colors.conf"') < 2 or "colors.conf.new." not in runtime:
        raise ThemeContractError(
            "Theme runtime must retain colors.conf loading and atomic persistence."
        )


def validate_repository_payload(repo: pathlib.Path) -> None:
    forbidden: list[str] = []
    payload_roots = (
        pathlib.PurePosixPath("data"),
        pathlib.PurePosixPath("win32/copy/share"),
    )
    for path in tracked_files(repo):
        lower = pathlib.PurePosixPath(str(path).casefold())
        if not any(lower == root or root in lower.parents for root in payload_roots):
            continue
        if (
            lower.suffix in {".hct", ".zct"}
            or lower.name == "colors.conf"
            or "themes" in lower.parts
        ):
            forbidden.append(str(path))
    if forbidden:
        raise ThemeContractError(
            "Repository-authored payload must not bundle an optional default theme: "
            + ", ".join(sorted(forbidden))
        )


def validate_wix_harvest(repo: pathlib.Path) -> None:
    forbidden: list[str] = []
    components = repo / "installer" / "Components"
    for path in sorted(components.glob("*.wxs")):
        try:
            root = ET.parse(path).getroot()
        except (OSError, ET.ParseError) as exc:
            raise ThemeContractError(f"Unable to parse {path}: {exc}") from exc
        for element in root.iter():
            for attribute in ("Source", "Include"):
                value = element.attrib.get(attribute)
                if not value:
                    continue
                normalized = value.replace("/", "\\").casefold()
                if (
                    normalized.endswith((".hct", ".zct", "\\colors.conf"))
                    or "\\share\\themes\\" in normalized
                    or "\\data\\themes\\" in normalized
                ):
                    forbidden.append(f"{path.relative_to(repo)}: {value}")
    if forbidden:
        raise ThemeContractError(
            "WiX must not harvest a bundled Fabulor/default desktop theme: "
            + ", ".join(forbidden)
        )


def validate(repo: pathlib.Path) -> None:
    validate_gtk3_theme_retirement(repo)
    validate_associations(repo)
    validate_import_contract(repo)
    validate_repository_payload(repo)
    validate_wix_harvest(repo)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo", type=pathlib.Path, default=pathlib.Path(__file__).resolve().parents[1]
    )
    args = parser.parse_args()
    repo = args.repo.resolve()
    try:
        validate(repo)
    except ThemeContractError as exc:
        print(f"Theme contract validation failed: {exc}", file=sys.stderr)
        return 1
    print(
        "Theme contract validated: .hct and colors.conf retained; .zct and bundled default themes excluded."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
