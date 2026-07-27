#!/usr/bin/env python3

import json
import pathlib
import tempfile
import unittest

import stage_plugin_hosts as hosts


TEST_TEMP_ROOT = pathlib.Path(__file__).resolve().parents[2] / "build" / "gtk4-plugin-host-tests"


class StagePluginHostsTests(unittest.TestCase):
    def setUp(self):
        TEST_TEMP_ROOT.mkdir(parents=True, exist_ok=True)
        self.temp_dir = tempfile.TemporaryDirectory(dir=TEST_TEMP_ROOT)
        self.base = pathlib.Path(self.temp_dir.name)
        self.roots = {}
        for name in hosts.SOURCE_NAMES:
            root = self.base / name
            root.mkdir()
            self.roots[name] = root
        self.contract_path = self.base / "contract.json"
        self.contract = {
            "schema_version": 1,
            "platform": "windows",
            "architecture": "x64",
            "dotnet_runtime_version": "8.0.29",
            "entries": [
                {
                    "source": "managed",
                    "path": "host.dll",
                    "destination": "Runtime/DotNet/host.dll",
                    "kind": "file",
                },
                {
                    "source": "payload",
                    "path": "_cffi_backend.*.pyd",
                    "destination": "Runtime/Python314",
                    "kind": "single_glob",
                },
                {
                    "source": "python",
                    "path": ".",
                    "destination": "Runtime/Python314",
                    "kind": "tree",
                },
                {
                    "source": "tcl",
                    "path": "lib",
                    "destination": "Runtime/Tcl/lib",
                    "kind": "tree",
                },
            ],
        }
        self._write("managed", "host.dll", b"managed")
        self._write("payload", "_cffi_backend.cp314-win_amd64.pyd", b"cffi")
        self._write("python", "python314.dll", b"python")
        self._write("tcl", "lib/init.tcl", b"tcl")

    def tearDown(self):
        self.temp_dir.cleanup()

    def _write(self, root, relative, content):
        path = self.roots[root].joinpath(*pathlib.PurePosixPath(relative).parts)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)

    def _load_contract(self):
        self.contract_path.write_text(json.dumps(self.contract), encoding="utf-8")
        return hosts.load_contract(self.contract_path)

    def test_stage_emits_exact_hashed_layout(self):
        output = self.base / "output"
        manifest = hosts.stage_plugin_hosts(
            self.roots, output, self._load_contract()
        )

        self.assertEqual(manifest["file_count"], 4)
        self.assertEqual(manifest["dotnet_runtime_version"], "8.0.29")
        self.assertTrue((output / "Runtime" / "Python314" / "python314.dll").is_file())
        recorded, files = hosts.load_manifest(
            output.joinpath(*pathlib.PurePosixPath(hosts.MANIFEST_RELATIVE).parts)
        )
        self.assertEqual(recorded["file_count"], len(files))
        self.assertIn("Runtime/Tcl/lib/init.tcl", files)

    def test_single_glob_requires_exactly_one_file(self):
        self._write("payload", "_cffi_backend.other.pyd", b"duplicate")
        with self.assertRaisesRegex(hosts.PluginHostStagingError, "Expected one"):
            hosts.collect_payload(self.roots, self._load_contract())

    def test_destination_collision_is_rejected(self):
        self.contract["entries"].append({
            "source": "managed",
            "path": "host.dll",
            "destination": "runtime/dotnet/HOST.dll",
            "kind": "file",
        })
        contract = self._load_contract()
        with self.assertRaisesRegex(hosts.PluginHostStagingError, "collision"):
            hosts.collect_payload(self.roots, contract)

    def test_unsafe_destination_is_rejected(self):
        self.contract["entries"][0]["destination"] = "../host.dll"
        with self.assertRaisesRegex(hosts.PluginHostStagingError, "Unsafe"):
            self._load_contract()

    def test_nonempty_output_is_rejected(self):
        output = self.base / "output"
        output.mkdir()
        (output / "owned.txt").write_text("existing", encoding="utf-8")
        with self.assertRaisesRegex(hosts.PluginHostStagingError, "not empty"):
            hosts.stage_plugin_hosts(self.roots, output, self._load_contract())

    def test_manifest_totals_are_enforced(self):
        output = self.base / "output"
        hosts.stage_plugin_hosts(self.roots, output, self._load_contract())
        manifest_path = output.joinpath(
            *pathlib.PurePosixPath(hosts.MANIFEST_RELATIVE).parts
        )
        value = json.loads(manifest_path.read_text(encoding="utf-8"))
        value["file_count"] += 1
        manifest_path.write_text(json.dumps(value), encoding="utf-8")
        with self.assertRaisesRegex(hosts.PluginHostStagingError, "totals"):
            hosts.load_manifest(manifest_path)

    def test_production_tcl_contract_is_an_explicit_embedded_runtime(self):
        contract = hosts.load_contract(hosts.DEFAULT_CONTRACT)
        tcl_entries = [
            entry for entry in contract["entries"] if entry["source"] == "tcl"
        ]
        selected = {
            (entry["path"], entry["destination"], entry["kind"])
            for entry in tcl_entries
        }

        self.assertIn(
            (
                "bin/tcl86t.dll",
                "Runtime/Tcl/bin/tcl86t.dll",
                "file",
            ),
            selected,
        )
        self.assertIn(
            (
                "lib/tcl8.6/init.tcl",
                "Runtime/Tcl/lib/tcl8.6/init.tcl",
                "file",
            ),
            selected,
        )
        self.assertIn(
            (
                "lib/tcl8/8.5/msgcat-1.6.1.tm",
                "Runtime/Tcl/lib/tcl8/8.5/msgcat-1.6.1.tm",
                "file",
            ),
            selected,
        )
        self.assertIn(
            (
                "lib/tcl8/8.6/http-2.9.8.tm",
                "Runtime/Tcl/lib/tcl8/8.6/http-2.9.8.tm",
                "file",
            ),
            selected,
        )
        self.assertIn(
            (
                "lib/tcl8.6/encoding",
                "Runtime/Tcl/lib/tcl8.6/encoding",
                "tree",
            ),
            selected,
        )
        self.assertIn(
            (
                "lib/tcl8.6/tzdata",
                "Runtime/Tcl/lib/tcl8.6/tzdata",
                "tree",
            ),
            selected,
        )
        self.assertNotIn(("bin", "Runtime/Tcl/bin", "tree"), selected)
        self.assertNotIn(("lib", "Runtime/Tcl/lib", "tree"), selected)
        self.assertNotIn(("lib/tcl8", "Runtime/Tcl/lib/tcl8", "tree"), selected)


if __name__ == "__main__":
    unittest.main()
