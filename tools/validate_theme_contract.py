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

    inno = read_text(repo / "win32" / "installer" / "zoitechat.iss.tt").casefold()
    if 'subkey: ".hct"' not in inno or 'subkey: ".zct"' in inno:
        raise ThemeContractError(
            "Legacy installer template must register .hct and must not register .zct."
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
    required = ('".hct"', '"colors.conf"', '"pevents.conf"', '"*.hct"')
    missing = [token for token in required if token not in preferences]
    if missing:
        raise ThemeContractError(
            f"Theme preferences are missing supported import tokens: {', '.join(missing)}"
        )
    if '".zct"' in preferences or '"*.zct"' in preferences:
        raise ThemeContractError("The active theme importer must not accept retired .zct files.")

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
