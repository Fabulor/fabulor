#!/usr/bin/env python3

import argparse
import importlib.metadata
import json
import pathlib
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
import zipfile


REQUIREMENTS = pathlib.Path(__file__).with_name("openai-sdk-requirements.txt")
INSTALL_PATH = pathlib.Path("Runtime/Python314/openai-sdk")
ARCHIVE_NAME = "Fabulor-OpenAI-SDK-Portable-x64.zip"
WIX_NAMESPACE = "http://wixtoolset.org/schemas/v4/wxs"
# These wheels predate or do not provide the standard Licence-Expression metadata.
LICENCE_EXPRESSIONS = {
    "certifi": "MPL-2.0",
    "colorama": "BSD-3-Clause",
    "distro": "Apache-2.0",
    "h11": "MIT",
    "httpx": "BSD-3-Clause",
    "openai": "Apache-2.0",
    "sniffio": "MIT OR Apache-2.0",
    "tqdm": "MPL-2.0 AND MIT",
}


def write_cache_cleanup(sdk, destination):
    # User-writable destinations can acquire bytecode after Setup finishes.
    # Remove only caches for sources provided by this feature.
    wix = ET.Element("Wix", xmlns=WIX_NAMESPACE)
    fragment = ET.SubElement(wix, "Fragment")
    component = ET.SubElement(
        fragment, "Component", Id="OpenAiSdkCacheCleanup",
        Directory="OPENAISDKDIR", Guid="046599BF-91AB-4ED8-AB21-9193AA19F827",
    )
    ET.SubElement(component, "CreateFolder")
    cache_dirs = {}
    for source in sorted(sdk.rglob("*.py")):
        cache_dir = source.relative_to(sdk).parent / "__pycache__"
        cache_dirs.setdefault(cache_dir, []).append(source.stem)
    for directory, modules in sorted(cache_dirs.items()):
        subdirectory = str(directory)
        for module in modules:
            for optimisation in ("", ".opt-1", ".opt-2"):
                ET.SubElement(
                    component, "RemoveFile", Subdirectory=subdirectory,
                    Name=f"{module}.cpython-314{optimisation}.pyc", On="uninstall",
                )
        ET.SubElement(component, "RemoveFolder", Subdirectory=subdirectory, On="uninstall")
    ET.indent(wix)
    ET.ElementTree(wix).write(destination, encoding="utf-8", xml_declaration=True)


def sdk_components(sdk):
    pins = dict(re.findall(
        r"^([a-z0-9-]+==[^ ]+) --hash=sha256:([a-f0-9]{64})$",
        REQUIREMENTS.read_text(encoding="utf-8"), flags=re.MULTILINE,
    ))
    components = []
    for distribution in importlib.metadata.distributions(path=[sdk]):
        metadata = distribution.metadata
        name = metadata["Name"]
        version = distribution.version
        identifier = re.sub(r"[-_.]+", "-", name).lower()
        metadata_file = next(
            path for path in distribution.files
            if path.name == "METADATA" and path.parent.name.endswith(".dist-info")
        )
        licences = sorted(
            (pathlib.PurePosixPath("openai-sdk") / path).as_posix()
            for path in distribution.files
            if path.is_relative_to(metadata_file.parent)
            and path.name.lower().startswith(("licen", "notice", "copying"))
        )
        if not licences:
            raise ValueError(f"Missing wheel licence for {name}")
        source = next(
            (url.strip() for label, _, url in
             (value.partition(",") for value in metadata.get_all("Project-URL", []))
             if label.strip().lower() in {"source", "source code", "repository", "github"}
             and url.strip().startswith("https://")),
            f"https://pypi.org/project/{identifier}/{version}/",
        )
        components.append({
            "id": f"openai-sdk-{identifier}",
            "name": name,
            "version": version,
            "type": "library",
            "scope": "optional",
            "licence": metadata.get("License-Expression") or LICENCE_EXPRESSIONS[identifier],
            "licence_files": licences,
            "source": source,
            "installed_paths": [(INSTALL_PATH / metadata_file.parent).as_posix()],
            "artifact_paths": [ARCHIVE_NAME],
            "distribution_sha256": pins.pop(f"{identifier}=={version}"),
        })
    if pins:
        raise ValueError(f"Missing locked wheels in staged SDK: {', '.join(pins)}")
    return sorted(components, key=lambda component: component["id"])


def stage(output, wheelhouse):
    if sys.platform != "win32" or sys.version_info[:2] != (3, 14) or struct.calcsize("P") != 8:
        raise ValueError("Stage the OpenAI SDK with Python 3.14 on Windows x64.")
    if output.exists():
        raise FileExistsError(f"Choose a fresh SDK staging directory: {output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=output.parent) as temporary:
        work = pathlib.Path(temporary)
        payload = work / "payload"
        sdk = payload / INSTALL_PATH
        sdk.parent.mkdir(parents=True)
        source = ["--index-url", "https://pypi.org/simple"]
        if wheelhouse is not None:
            source = ["--no-index", "--find-links", str(wheelhouse.resolve(strict=True))]
        subprocess.run([
            sys.executable, "-m", "pip", "--isolated", "--disable-pip-version-check",
            "install", "--only-binary", ":all:", "--require-hashes", "--ignore-installed",
            "--no-compile", "--platform", "win_amd64", "--python-version", "3.14",
            "--implementation", "cp", "--abi", "cp314", "--target", str(sdk),
            "--requirement", str(REQUIREMENTS), *source,
        ], check=True)
        if (sdk / "bin").is_dir():
            shutil.rmtree(sdk / "bin")

        components = sdk_components(sdk)
        (sdk / "fabulor-openai-sdk.json").write_text(
            json.dumps(components, indent=2) + "\n", encoding="utf-8",
        )
        write_cache_cleanup(sdk, payload / "openai-sdk-cache.wxs")
        with zipfile.ZipFile(payload / ARCHIVE_NAME, "w", zipfile.ZIP_DEFLATED) as archive:
            for file in sorted(sdk.rglob("*")):
                if file.is_file():
                    archive.write(file, file.relative_to(payload).as_posix())
        payload.rename(output)
    print(f"Staged {len(components)} SDK packages in {output}")


def main():
    parser = argparse.ArgumentParser(description="Stage the optional Windows x64 OpenAI SDK")
    parser.add_argument("--output", type=pathlib.Path, required=True)
    parser.add_argument("--wheelhouse", type=pathlib.Path)
    args = parser.parse_args()
    try:
        stage(args.output.resolve(), args.wheelhouse)
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"OpenAI SDK staging failed: {error}\n")


if __name__ == "__main__":
    main()
