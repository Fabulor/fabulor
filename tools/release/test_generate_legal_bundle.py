#!/usr/bin/env python3

import json
import pathlib
import shutil
import unittest

import generate_legal_bundle as legal


class LegalBundleTests(unittest.TestCase):
    def setUp(self):
        repository = pathlib.Path(__file__).parents[2]
        self.test_root = repository / "build" / "legal-bundle-tests" / self._testMethodName
        if self.test_root.exists():
            shutil.rmtree(self.test_root)
        self.test_root.mkdir(parents=True)

    def tearDown(self):
        shutil.rmtree(self.test_root, ignore_errors=True)

    def test_repository_manifest_generates_deterministic_cyclonedx(self):
        repository = pathlib.Path(__file__).parents[2]
        manifest = repository / "third-party" / "components.json"
        version_props = repository / "installer" / "Directory.Build.props"
        licences = repository / "third-party" / "licenses"

        first = self.test_root / "first"
        second = self.test_root / "second"
        legal.generate(manifest, version_props, licences, first)
        legal.generate(manifest, version_props, licences, second)
        first_files = {
            path.relative_to(first): path.read_bytes()
            for path in first.rglob("*") if path.is_file()
        }
        second_files = {
            path.relative_to(second): path.read_bytes()
            for path in second.rglob("*") if path.is_file()
        }
        self.assertEqual(first_files, second_files)

        sbom_path = first / "Fabulor-1.0.6.cdx.json"
        sbom = json.loads(sbom_path.read_text(encoding="utf-8"))
        self.assertEqual(sbom["bomFormat"], "CycloneDX")
        self.assertEqual(sbom["specVersion"], "1.6")
        self.assertEqual(len(sbom["components"]), 41)
        self.assertNotIn("timestamp", sbom["metadata"])

    def test_missing_licence_evidence_fails_closed(self):
        manifest = {
            "schema_version": 1,
            "product": {
                "name": "Fabulor", "type": "application",
                "homepage": "https://github.com/Fabulor/fabulor",
                "licence": "GPL-3.0-only",
            },
            "components": [{
                "id": "sample", "name": "Sample", "version": "1.0",
                "type": "library", "scope": "required", "licence": "MIT",
                "licence_files": ["missing"], "source": "https://example.invalid/source",
                "installed_paths": ["sample.dll"],
            }],
        }
        path = self.test_root / "components.json"
        path.write_text(json.dumps(manifest), encoding="utf-8")
        loaded = legal.load_manifest(path)
        with self.assertRaises(legal.LegalBundleError):
            legal.validate_licences(loaded, self.test_root)

    def test_output_outside_build_directory_is_rejected(self):
        repository = pathlib.Path(__file__).parents[2]
        with self.assertRaises(legal.LegalBundleError):
            legal.generate(
                repository / "third-party" / "components.json",
                repository / "installer" / "Directory.Build.props",
                repository / "third-party" / "licenses",
                repository / "third-party" / "unsafe-output",
            )


if __name__ == "__main__":
    unittest.main()
