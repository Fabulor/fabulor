#!/usr/bin/env python3

import argparse
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile


PROBE_NAME = "fabulor-gtk4-runtime-root-probe.exe"


class RuntimeRootProbeError(Exception):
    pass


def run_probe(probe, cwd, env):
    return subprocess.run(
        [str(probe)],
        cwd=cwd,
        env=env,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
    )


def require_failure(probe, cwd, env, scenario):
    completed = run_probe(probe, cwd, env)
    if completed.returncode == 0:
        raise RuntimeRootProbeError(f"Probe did not reject {scenario}")


def validate_runtime_root(candidate_root):
    probe = (candidate_root / PROBE_NAME).resolve(strict=True)
    runtime_root = (candidate_root / "Runtime" / "GTK4").resolve(strict=True)
    expected_dll = runtime_root / "bin" / "gtk-4-1.dll"
    expected_dll.resolve(strict=True)

    env = os.environ.copy()
    windows_root = pathlib.Path(
        env.get("SystemRoot") or env.get("WINDIR") or r"C:\Windows"
    )
    env["SystemRoot"] = str(windows_root)
    env["PATH"] = str(windows_root / "System32")

    with tempfile.TemporaryDirectory(prefix="fabulor-gtk4-runtime-root-") as temporary:
        temporary_root = pathlib.Path(temporary)
        decoy_cwd = temporary_root / "decoy-cwd"
        decoy_cwd.mkdir()
        (decoy_cwd / "gtk-4-1.dll").write_bytes(b"not a DLL\n")

        completed = run_probe(probe, decoy_cwd, env)
        if completed.returncode != 0:
            raise RuntimeRootProbeError(
                f"Executable-relative GTK4 probe failed:\n{completed.stdout}"
            )
        if str(expected_dll).casefold() not in completed.stdout.casefold():
            raise RuntimeRootProbeError(
                "Probe did not report the expected packaged GTK4 module path"
            )

        missing_root = temporary_root / "missing-root"
        missing_root.mkdir()
        missing_probe = missing_root / PROBE_NAME
        shutil.copy2(probe, missing_probe)
        require_failure(missing_probe, decoy_cwd, env, "a missing runtime root")

        reparse_root = temporary_root / "reparse-root"
        (reparse_root / "Runtime").mkdir(parents=True)
        reparse_probe = reparse_root / PROBE_NAME
        shutil.copy2(probe, reparse_probe)
        junction = reparse_root / "Runtime" / "GTK4"
        command = [
            env.get("ComSpec", str(windows_root / "System32" / "cmd.exe")),
            "/d",
            "/c",
            "mklink",
            "/J",
            str(junction),
            str(runtime_root),
        ]
        linked = subprocess.run(
            command, check=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, encoding="utf-8", errors="replace"
        )
        if linked.returncode != 0:
            raise RuntimeRootProbeError(
                f"Unable to create the reparse-point test fixture:\n{linked.stdout}"
            )
        try:
            require_failure(reparse_probe, decoy_cwd, env, "a reparse-point runtime root")
        finally:
            os.rmdir(junction)


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Test the executable-relative Windows GTK4 runtime boundary"
    )
    parser.add_argument("--candidate-root", type=pathlib.Path, required=True)
    return parser.parse_args(argv)


def main(argv=None):
    if sys.platform != "win32":
        print("GTK4 runtime-root probe tests require Windows", file=sys.stderr)
        return 1

    args = parse_args(argv)
    try:
        validate_runtime_root(args.candidate_root.resolve(strict=True))
    except (OSError, KeyError, RuntimeRootProbeError) as exc:
        print(f"GTK4 runtime-root probe validation failed: {exc}", file=sys.stderr)
        return 1

    print("GTK4 runtime-root probe validated: decoy, missing, and reparse cases passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
