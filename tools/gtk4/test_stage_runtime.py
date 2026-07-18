#!/usr/bin/env python3

import json
import pathlib
import tempfile
import unittest

import stage_runtime


TEST_TEMP_ROOT = pathlib.Path(__file__).resolve().parents[2] / "build" / "gtk4-staging-tests"


class StageGtk4RuntimeTests(unittest.TestCase):
    def setUp(self):
        TEST_TEMP_ROOT.mkdir(parents=True, exist_ok=True)
        self.temp_dir = tempfile.TemporaryDirectory(dir=TEST_TEMP_ROOT)
        self.base = pathlib.Path(self.temp_dir.name)
        self.root = self.base / "root"
        self.root.mkdir()
        self.contract_path = self.base / "runtime.json"
        self.source_contract_path = self.base / "source.json"
        self.contract = {
            "schema_version": 1,
            "platform": "windows",
            "architecture": "x64",
            "source_contract": "source.json",
            "files": [
                "bin/gtk-4-1.dll",
                "lib/gdk-pixbuf-2.0/2.10.0/loaders.cache",
            ],
            "trees": ["share/icons"],
            "text_normalizations": {
                "lib/gdk-pixbuf-2.0/2.10.0/loaders.cache": "gdk-pixbuf-loader-cache"
            },
            "forbidden_suffixes": [".h", ".pdb"],
        }
        self.source_contract_path.write_text(
            json.dumps({"source": {"sha256": "a" * 64}}), encoding="utf-8"
        )
        self._write("bin/gtk-4-1.dll", b"runtime")
        self._write(
            "lib/gdk-pixbuf-2.0/2.10.0/loaders.cache",
            b"# LoaderDir = C:\\build\\machine\nloader\n",
        )
        self._write("share/icons/icon.png", b"icon")

    def tearDown(self):
        self.temp_dir.cleanup()

    def _write(self, relative, data):
        path = self.root.joinpath(*pathlib.PurePosixPath(relative).parts)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)

    def _save_contract(self):
        self.contract_path.write_text(json.dumps(self.contract), encoding="utf-8")
        return stage_runtime.load_contract(self.contract_path)

    def test_stage_emits_normalized_hashed_manifest(self):
        contract = self._save_contract()
        output = self.base / "output"

        manifest = stage_runtime.stage_runtime(
            self.root, output, contract, self.contract_path
        )

        self.assertEqual(manifest["file_count"], 3)
        self.assertEqual(manifest["source_archive_sha256"], "a" * 64)
        cache = output / "lib" / "gdk-pixbuf-2.0" / "2.10.0" / "loaders.cache"
        self.assertNotIn("C:\\build", cache.read_text(encoding="utf-8"))
        recorded = json.loads((output / stage_runtime.MANIFEST_NAME).read_text())
        self.assertEqual(recorded["files"], manifest["files"])

    def test_missing_exact_file_is_rejected(self):
        contract = self._save_contract()
        (self.root / "bin" / "gtk-4-1.dll").unlink()
        with self.assertRaisesRegex(stage_runtime.StagingError, "Missing runtime"):
            stage_runtime.collect_payload(self.root, contract)

    def test_forbidden_tree_file_is_rejected(self):
        self._write("share/icons/debug.pdb", b"debug")
        contract = self._save_contract()
        with self.assertRaisesRegex(stage_runtime.StagingError, "forbidden files"):
            stage_runtime.collect_payload(self.root, contract)

    def test_nonempty_output_is_rejected(self):
        contract = self._save_contract()
        output = self.base / "output"
        output.mkdir()
        (output / "owned.txt").write_text("existing", encoding="utf-8")
        with self.assertRaisesRegex(stage_runtime.StagingError, "not empty"):
            stage_runtime.stage_runtime(self.root, output, contract, self.contract_path)

    def test_normalization_failure_leaves_no_partial_output(self):
        contract = self._save_contract()
        self._write(
            "lib/gdk-pixbuf-2.0/2.10.0/loaders.cache", b"unexpected cache format\n"
        )
        output = self.base / "output"
        with self.assertRaisesRegex(stage_runtime.StagingError, "Unexpected GDK"):
            stage_runtime.stage_runtime(self.root, output, contract, self.contract_path)
        self.assertFalse(output.exists())

    def test_unsafe_contract_path_is_rejected(self):
        self.contract["files"][0] = "../gtk-4-1.dll"
        self.contract_path.write_text(json.dumps(self.contract), encoding="utf-8")
        with self.assertRaisesRegex(stage_runtime.StagingError, "Unsafe"):
            stage_runtime.load_contract(self.contract_path)

    def test_invalid_source_identity_is_rejected_without_output(self):
        contract = self._save_contract()
        self.source_contract_path.write_text(
            json.dumps({"source": {"sha256": "not-a-hash"}}), encoding="utf-8"
        )
        output = self.base / "output"
        with self.assertRaisesRegex(stage_runtime.StagingError, "lowercase SHA-256"):
            stage_runtime.stage_runtime(self.root, output, contract, self.contract_path)
        self.assertFalse(output.exists())


if __name__ == "__main__":
    unittest.main()
