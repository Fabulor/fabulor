#!/usr/bin/env python3

import pathlib
import shutil
import subprocess
import tempfile
import unittest


REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
SCRIPT = REPO_ROOT / "tools" / "clean-development-output.ps1"
POWERSHELL = shutil.which("pwsh") or shutil.which("powershell")


class DevelopmentOutputCleanupTests(unittest.TestCase):
    def setUp(self):
        if POWERSHELL is None:
            self.skipTest("PowerShell is required")

        self.temp_dir = tempfile.TemporaryDirectory(
            prefix=".cleanup-test-",
            dir=REPO_ROOT,
        )
        self.repo = pathlib.Path(self.temp_dir.name) / "fabulor"
        (self.repo / ".git").mkdir(parents=True)
        (self.repo / "tools").mkdir()
        shutil.copy2(SCRIPT, self.repo / "tools" / SCRIPT.name)

        removable = (
            "build/output.bin",
            "installer/obj/cache.dat",
            "installer/build/package.tmp",
            "installer/Bootstrapper/obj/bundle.tmp",
            "installer/UX/obj/ui.tmp",
            "src/managed/Host/bin/host.dll",
            "src/managed/Host/obj/host.obj",
            "samples/plugins/example/bin/sample.dll",
            "samples/plugins/example/obj/sample.obj",
            "plugins/python/__pycache__/module.pyc",
        )
        retained = (
            "README.md",
            "Runtime/GTK4/bin/gtk-4-1.dll",
            "Runtime/GTK4/lib/__pycache__/runtime.pyc",
            ".vscode/settings.json",
            "dos2unix.exe",
            "installer/bin/x64/Release/FabulorSetup.exe",
            "installer/UX/bin/Release/Fabulor.Bootstrapper.dll",
        )
        for relative in removable + retained:
            path = self.repo / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(relative, encoding="utf-8")

    def tearDown(self):
        self.temp_dir.cleanup()

    def run_cleanup(self, *arguments):
        return subprocess.run(
            [
                POWERSHELL,
                "-NoLogo",
                "-NoProfile",
                "-File",
                str(self.repo / "tools" / SCRIPT.name),
                *arguments,
            ],
            check=False,
            capture_output=True,
            text=True,
        )

    def test_default_mode_is_a_non_destructive_preview(self):
        result = self.run_cleanup()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("Mode: preview", result.stdout)
        self.assertIn("WOULD REMOVE", result.stdout)
        self.assertTrue((self.repo / "build" / "output.bin").is_file())
        self.assertTrue((self.repo / "Runtime" / "GTK4" / "bin" / "gtk-4-1.dll").is_file())
        self.assertTrue((self.repo / "Runtime" / "GTK4" / "lib" / "__pycache__" / "runtime.pyc").is_file())

    def test_apply_removes_intermediates_but_retains_protected_and_installer_outputs(self):
        result = self.run_cleanup("-Apply")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse((self.repo / "build").exists())
        self.assertFalse((self.repo / "installer" / "obj").exists())
        self.assertFalse((self.repo / "src" / "managed" / "Host" / "bin").exists())
        self.assertFalse((self.repo / "plugins" / "python" / "__pycache__").exists())
        self.assertTrue((self.repo / "README.md").is_file())
        self.assertTrue((self.repo / "Runtime" / "GTK4" / "bin" / "gtk-4-1.dll").is_file())
        self.assertTrue((self.repo / ".vscode" / "settings.json").is_file())
        self.assertTrue((self.repo / "dos2unix.exe").is_file())
        self.assertTrue((self.repo / "installer" / "bin" / "x64" / "Release" / "FabulorSetup.exe").is_file())
        self.assertTrue((self.repo / "installer" / "UX" / "bin" / "Release" / "Fabulor.Bootstrapper.dll").is_file())

    def test_installer_artifacts_require_an_explicit_second_switch(self):
        result = self.run_cleanup("-Apply", "-IncludeInstallerArtifacts")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse((self.repo / "installer" / "bin").exists())
        self.assertFalse((self.repo / "installer" / "UX" / "bin").exists())
        self.assertTrue((self.repo / "Runtime" / "GTK4" / "bin" / "gtk-4-1.dll").is_file())


if __name__ == "__main__":
    unittest.main()
