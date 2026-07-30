import hashlib
import json
import pathlib
import tempfile
import unittest

import stage_production_support as support


class ProductionSupportTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self.temporary.name)
        self.roots = {
            name: self.root / name
            for name in support.SOURCE_NAMES
        }
        for root in self.roots.values():
            root.mkdir()
        files = {
            "dependency": [
                "bin/cert.pem",
                "bin/libcrypto-3-x64.dll",
                "bin/libssl-3-x64.dll",
            ],
            "python": ["Lib/site-packages/_cffi_backend.test.pyd"],
            "repository": [
                "plugins/python/xchat.py",
                "plugins/python/hexchat.py",
                "plugins/python/_fabulor_manifest.py",
                "plugins/python/fabulor.py",
                "plugins/python/_fabulor.py",
            ],
        }
        for source, relatives in files.items():
            for relative in relatives:
                path = self.roots[source].joinpath(
                    *pathlib.PurePosixPath(relative).parts
                )
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(relative.encode("ascii"))
        self.contract = support.load_contract(support.DEFAULT_CONTRACT)

    def tearDown(self):
        self.temporary.cleanup()

    def test_exact_hashed_payload_is_staged(self):
        output = self.root / "output"
        manifest = support.stage(self.roots, output, self.contract)

        self.assertEqual(manifest["file_count"], 9)
        paths = [entry["path"] for entry in manifest["files"]]
        self.assertEqual(paths, sorted(paths, key=str.casefold))
        self.assertIn("python/fabulor.py", paths)
        self.assertIn("python/_fabulor.py", paths)
        self.assertNotIn("python/zoitechat.py", paths)
        self.assertNotIn("python/_zoitechat.py", paths)
        self.assertIn("_cffi_backend.test.pyd", paths)
        manifest_path = output / support.MANIFEST_NAME
        self.assertTrue(manifest_path.is_file())
        recorded = json.loads(manifest_path.read_text(encoding="utf-8"))
        target = output / "cert.pem"
        cert = next(entry for entry in recorded["files"] if entry["path"] == "cert.pem")
        self.assertEqual(cert["sha256"], hashlib.sha256(target.read_bytes()).hexdigest())

    def test_nonempty_output_is_rejected(self):
        output = self.root / "output"
        output.mkdir()
        (output / "unexpected").write_text("x", encoding="ascii")

        with self.assertRaises(support.ProductionSupportError):
            support.stage(self.roots, output, self.contract)

    def test_single_glob_requires_one_match(self):
        second = self.roots["python"] / "Lib/site-packages/_cffi_backend.other.pyd"
        second.write_bytes(b"duplicate")

        with self.assertRaises(support.ProductionSupportError):
            support.collect_payload(self.roots, self.contract)

    def test_duplicate_contract_fields_are_rejected(self):
        contract = self.root / "duplicate.json"
        contract.write_text(
            '{"schema_version":1,"schema_version":1,"platform":"windows",'
            '"architecture":"x64","entries":[]}',
            encoding="utf-8",
        )

        with self.assertRaises(support.ProductionSupportError):
            support.load_contract(contract)

    def test_destination_collision_is_rejected(self):
        contract = {**self.contract, "entries": list(self.contract["entries"])}
        contract["entries"].append({
            "source": "repository",
            "path": "plugins/python/fabulor.py",
            "destination": "CERT.PEM",
            "kind": "file",
        })

        with self.assertRaises(support.ProductionSupportError):
            support.collect_payload(self.roots, contract)


if __name__ == "__main__":
    unittest.main()
