#!/usr/bin/env python3

import pathlib
import re
import unittest


REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
ACTIVE_SOURCE_ROOTS = (
    REPO_ROOT / "src",
    REPO_ROOT / "plugins",
    REPO_ROOT / "win32",
)
ACTIVE_SUFFIXES = {
    ".c",
    ".cpp",
    ".h",
    ".filters",
    ".ps1",
    ".py",
    ".tt",
    ".vcxproj",
    ".xml",
}
RETIRED_PREFIX = "zoite" "chat"
RETIRED_PRODUCT = "Zoite" "Chat"
RETIRED_INTERNAL_PATTERNS = (
    re.compile(rf"\b{RETIRED_PREFIX}_", re.IGNORECASE),
    re.compile(rf"\b{RETIRED_PREFIX}-", re.IGNORECASE),
    re.compile(rf"\b_{RETIRED_PREFIX}_marshal\b", re.IGNORECASE),
)
ALLOWED_COMPATIBILITY_FILES = {
    REPO_ROOT / "plugins" / "python" / "_fabulor_manifest.py",
    REPO_ROOT / "src" / "common" / "secretstore.c",
}


class InternalBrandingContractTests(unittest.TestCase):
    def test_core_source_uses_fabulor_filenames(self):
        common = REPO_ROOT / "src" / "common"
        for name in ("fabulor.c", "fabulor.h", "fabulorc.h"):
            with self.subTest(name=name):
                self.assertTrue((common / name).is_file())
        for name in (
            f"{RETIRED_PREFIX}.c",
            f"{RETIRED_PREFIX}.h",
            f"{RETIRED_PREFIX}c.h",
        ):
            with self.subTest(name=name):
                self.assertFalse((common / name).exists())

    def test_active_internal_prefixes_are_retired(self):
        failures = []
        for root in ACTIVE_SOURCE_ROOTS:
            for path in root.rglob("*"):
                if (
                    not path.is_file()
                    or path.suffix.lower() not in ACTIVE_SUFFIXES
                    or path in ALLOWED_COMPATIBILITY_FILES
                    or path.name.startswith("test_")
                    or "tests" in path.parts
                ):
                    continue
                source = path.read_text(encoding="utf-8")
                for pattern in RETIRED_INTERNAL_PATTERNS:
                    if pattern.search(source):
                        failures.append(str(path.relative_to(REPO_ROOT)))
                        break
        self.assertEqual(failures, [])

    def test_build_configuration_has_no_retired_version_fallback(self):
        version_script = (REPO_ROOT / "win32" / "version-template.ps1").read_text(
            encoding="utf-8"
        )
        config_template = (REPO_ROOT / "win32" / "config.h.tt").read_text(
            encoding="utf-8"
        )
        self.assertNotIn(f"{RETIRED_PRODUCT}SemVer", version_script)
        self.assertIn("FABULORLIBDIR", config_template)
        self.assertIn("FABULORSHAREDIR", config_template)


if __name__ == "__main__":
    unittest.main()
