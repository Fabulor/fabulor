#!/usr/bin/env python3

import hashlib
import json
import pathlib
import struct
import tempfile
import unittest

import validate_root


TEST_TEMP_ROOT = pathlib.Path(__file__).resolve().parents[2] / "build" / "gtk4-validator-tests"


class ValidateGtk4RootTests(unittest.TestCase):
    def setUp(self):
        self.contract = validate_root.load_contract(validate_root.DEFAULT_CONTRACT)
        TEST_TEMP_ROOT.mkdir(parents=True, exist_ok=True)
        self.temp_dir = tempfile.TemporaryDirectory(dir=TEST_TEMP_ROOT)
        self.root = pathlib.Path(self.temp_dir.name) / "gtk4"
        self._write_valid_root()

    def tearDown(self):
        self.temp_dir.cleanup()

    def _write(self, relative, content=b""):
        target = self.root.joinpath(*pathlib.PurePosixPath(relative).parts)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(content)

    def _write_valid_root(self):
        for relative in self.contract["required_files"]:
            self._write(relative)

        self._write(
            "include/gtk-4.0/gtk/gtkversion.h",
            b"#define GTK_MAJOR_VERSION (4)\n"
            b"#define GTK_MINOR_VERSION (22)\n"
            b"#define GTK_MICRO_VERSION (4)\n",
        )
        self._write(
            "lib/glib-2.0/include/glibconfig.h",
            b"#define GLIB_MAJOR_VERSION 2\n"
            b"#define GLIB_MINOR_VERSION 88\n"
            b"#define GLIB_MICRO_VERSION 0\n"
            b"#define GLIB_SIZEOF_VOID_P 8\n",
        )
        self._write(
            "lib/pkgconfig/gtk4.pc",
            b"prefix=C:/original/root\n"
            b"gtk_host=x86_64-windows\n"
            b"Version: 4.22.4\n",
        )
        self._write(
            "lib/pkgconfig/glib-2.0.pc",
            b"prefix=C:/original/root\nVersion: 2.88.0\n",
        )

        pe = bytearray(128)
        pe[0:2] = b"MZ"
        struct.pack_into("<I", pe, 0x3C, 64)
        pe[64:68] = b"PE\0\0"
        struct.pack_into("<H", pe, 68, 0x8664)
        self._write("bin/gtk-4-1.dll", pe)
        self._write("bin/glib-2.0-0.dll", pe)

    def test_valid_root_reports_identity(self):
        result = validate_root.validate_root(self.root, self.contract)
        self.assertEqual(result["gtk_version"], "4.22.4")
        self.assertEqual(result["glib_version"], "2.88.0")
        self.assertEqual(result["architecture"], "x64")
        self.assertEqual(
            result["required_file_count"], len(self.contract["required_files"])
        )

    def test_missing_required_file_is_rejected(self):
        (self.root / "lib" / "gtk-4.lib").unlink()
        with self.assertRaisesRegex(validate_root.ValidationError, "Missing GTK4"):
            validate_root.validate_root(self.root, self.contract)

    def test_wrong_architecture_is_rejected(self):
        for name in ("gtk-4-1.dll", "glib-2.0-0.dll"):
            dll = self.root / "bin" / name
            data = bytearray(dll.read_bytes())
            struct.pack_into("<H", data, 68, 0x014C)
            dll.write_bytes(data)
        with self.assertRaisesRegex(validate_root.ValidationError, "identity mismatch"):
            validate_root.validate_root(self.root, self.contract)

    def test_gtk3_marker_is_rejected(self):
        self._write("include/gtk-3.0/gtk/gtk.h", b"legacy")
        with self.assertRaisesRegex(validate_root.ValidationError, "GTK3 build markers"):
            validate_root.validate_root(self.root, self.contract)

    def test_mixed_runtime_architectures_are_rejected(self):
        dll = self.root / "bin" / "glib-2.0-0.dll"
        data = bytearray(dll.read_bytes())
        struct.pack_into("<H", data, 68, 0x014C)
        dll.write_bytes(data)
        with self.assertRaisesRegex(validate_root.ValidationError, "do not match"):
            validate_root.validate_root(self.root, self.contract)

    def test_contract_rejects_duplicate_json_fields(self):
        duplicate_contract = pathlib.Path(self.temp_dir.name) / "duplicate.json"
        duplicate_contract.write_text(
            json.dumps(self.contract)[:-1] + ', "gtk_version": "4.22.4"}',
            encoding="utf-8",
        )
        with self.assertRaisesRegex(validate_root.ValidationError, "Duplicate"):
            validate_root.load_contract(duplicate_contract)

    def test_archive_hash_and_size_are_validated(self):
        archive = pathlib.Path(self.temp_dir.name) / "gtk4.zip"
        payload = b"pinned GTK4 archive fixture"
        archive.write_bytes(payload)
        contract = json.loads(json.dumps(self.contract))
        contract["source"]["size_bytes"] = len(payload)
        contract["source"]["sha256"] = hashlib.sha256(payload).hexdigest()

        result = validate_root.validate_archive(archive, contract)

        self.assertEqual(result["archive_bytes"], len(payload))
        self.assertEqual(result["archive_sha256"], contract["source"]["sha256"])

    def test_archive_hash_mismatch_is_rejected(self):
        archive = pathlib.Path(self.temp_dir.name) / "gtk4.zip"
        payload = b"unexpected payload"
        archive.write_bytes(payload)
        contract = json.loads(json.dumps(self.contract))
        contract["source"]["size_bytes"] = len(payload)

        with self.assertRaisesRegex(validate_root.ValidationError, "SHA-256 mismatch"):
            validate_root.validate_archive(archive, contract)


if __name__ == "__main__":
    unittest.main()
