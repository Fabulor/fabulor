#!/usr/bin/env python3

import hashlib
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
        version, _, _, sbom_name = legal.generate(manifest, version_props, licences, first)
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

        sbom_path = first / sbom_name
        sbom = json.loads(sbom_path.read_text(encoding="utf-8"))
        self.assertEqual(sbom["bomFormat"], "CycloneDX")
        self.assertEqual(sbom["specVersion"], "1.6")
        self.assertEqual(sbom["metadata"]["component"]["version"], version)
        self.assertEqual(len(sbom["components"]), 41)
        self.assertNotIn("timestamp", sbom["metadata"])

    def test_sdk_inventory_is_merged_into_legal_bundle(self):
        repository = pathlib.Path(__file__).parents[2]
        manifest = repository / "third-party" / "components.json"
        version_props = repository / "installer" / "Directory.Build.props"
        licences = repository / "third-party" / "licenses"
        sdk_root = self.test_root / "staged-sdk"
        python_root = sdk_root / "Runtime" / "Python314"
        licence_path = "openai-sdk/sample.dist-info/licenses/LICENSE"
        licence_text = b"Sample SDK component licence.\n"
        evidence = python_root / licence_path
        evidence.parent.mkdir(parents=True)
        evidence.write_bytes(licence_text)
        inventory = python_root / "openai-sdk" / "fabulor-openai-sdk.json"
        wheel_digest = hashlib.sha256(b"sample SDK wheel").hexdigest()
        component = {
            "id": "sdk-test-sample",
            "name": "Sample SDK",
            "version": "1.0",
            "type": "library",
            "scope": "optional",
            "licence": "MIT",
            "licence_files": [licence_path],
            "source": "https://example.invalid/sdk",
            "installed_paths": ["Runtime/Python314/openai-sdk/sample"],
            "distribution_sha256": wheel_digest,
        }
        inventory.write_text(json.dumps([component]), encoding="utf-8")

        first = self.test_root / "first"
        second = self.test_root / "second"
        _, _, _, sbom_name = legal.generate(manifest, version_props, licences, first, sdk_root=sdk_root)
        legal.generate(manifest, version_props, licences, second, sdk_root=sdk_root)
        for name in (sbom_name, "THIRD-PARTY-NOTICES.md"):
            self.assertEqual((first / name).read_bytes(), (second / name).read_bytes())

        sbom = json.loads((first / sbom_name).read_text(encoding="utf-8"))
        sdk_entry = next(item for item in sbom["components"] if item["bom-ref"] == "component:sdk-test-sample@1.0")
        self.assertEqual(sdk_entry["scope"], "optional")
        self.assertEqual(sdk_entry["hashes"], [{"alg": "SHA-256", "content": wheel_digest}])
        self.assertIn(sdk_entry["bom-ref"], sbom["dependencies"][0]["dependsOn"])
        self.assertEqual((first / "licenses" / licence_path).read_bytes(), licence_text)
        notices = (first / "THIRD-PARTY-NOTICES.md").read_text(encoding="utf-8")
        self.assertIn(f"| `licenses/{licence_path}` | `{hashlib.sha256(licence_text).hexdigest()}` |", notices)

        existing_id = legal.load_manifest(manifest)["components"][0]["id"]
        for invalid_component, error in (
            (dict(component, licence_files=["openai-sdk/missing"]), "missing or unsafe licence evidence"),
            (dict(component, id=existing_id), "component ids must be unique"),
        ):
            with self.subTest(error=error):
                inventory.write_text(json.dumps([invalid_component]), encoding="utf-8")
                output = self.test_root / "rejected"
                with self.assertRaisesRegex(legal.LegalBundleError, error):
                    legal.generate(manifest, version_props, licences, output, sdk_root=sdk_root)
                self.assertFalse(output.exists())

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
