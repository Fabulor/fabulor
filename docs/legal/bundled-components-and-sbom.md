# Bundled Components, Licences, And SBOM

Status: release control implemented on 2026-08-03
Scope: Fabulor 1.0.6 Windows MSI and bootstrapper payload

This is an engineering record of redistributed components and licence
evidence. It is not legal advice.

## Authoritative Inventory

[`../../third-party/components.json`](../../third-party/components.json) is
the machine-readable source of truth for the Windows package. It currently
identifies 41 components, including:

- the allowlisted GTK4 runtime and its transitive DLLs;
- OpenSSL and the Mozilla CA certificate bundle;
- Enchant and its WinSpell provider;
- the Python, CFFI, Tcl, and .NET plugin runtimes;
- the WiX Burn engine and bootstrapper API embedded in `FabulorSetup.exe`;
- Noto Color Emoji, Adwaita, ISO code data, and Flagpedia flag assets; and
- the separately licensed FiSHLiM native plugin.

Each entry records a stable identifier, component type, release or snapshot
version, required or optional scope, licence expression, upstream source,
installed paths, and the licence evidence shipped with Fabulor. Fixed external
release artefacts also record SHA-256 values where the build consumes the
archive directly.

`repository-snapshot` means the redistributed data is pinned by the Fabulor
source commit but the inherited import did not preserve a trustworthy upstream
release number. This is explicit rather than substituting an invented version.

## Licence Evidence

Verbatim upstream texts are retained in
[`../../third-party/licenses`](../../third-party/licenses). The release
generator rejects missing, empty, duplicate, or path-escaping evidence before
it creates an installer payload.

The generated legal bundle contains:

- `THIRD-PARTY-NOTICES.md`, a human-readable component and evidence table;
- `licenses/`, the exact evidence files and their SHA-256 checksums; and
- `Fabulor-<version>.cdx.json`, the CycloneDX 1.6 SBOM.

WiX installs that bundle beneath:

```text
%ProgramFiles%\Fabulor\share\doc\fabulor\third-party
```

The Fabulor GPLv3 text remains separately installed at
`share\doc\fabulor\Licence.md`.

The WiX Toolset source licence and the agreement accompanying its binary
NuGet release are both preserved because the Burn engine and bootstrapper API
are carried by `FabulorSetup.exe`, rather than installed as standalone files
by the MSI.

## Deterministic Generation

Run the same generator used by the Windows workflow:

```powershell
python tools\release\generate_legal_bundle.py --output build\release-legal
```

The generator reads the product version from
`installer\Directory.Build.props`. It deliberately omits a wall-clock
timestamp and derives the CycloneDX serial number from the product version and
component-manifest digest, so identical inputs produce byte-identical output.

The focused validation is:

```powershell
python tools\release\test_generate_legal_bundle.py
```

## Release Enforcement

The Windows workflow:

1. pins CFFI 2.1.0;
2. verifies the Tcl 8.6.16 MSI and Enchant 2.8.19 source archive hashes;
3. generates and tests the legal bundle before WiX runs;
4. makes WiX fail if the notices or versioned SBOM are absent;
5. verifies every inventory path against the files and populated directories
   in the built MSI;
6. installs the complete bundle in the MSI; and
7. uploads the legal bundle as a separate workflow artefact.

The GTK4 archive, Python embeddable archive, Mozilla CA bundle, Tcl MSI, and
Enchant source archive are checksum-controlled. OpenSSL is resolved through
the repository's fixed vcpkg baseline. The .NET runtime is fixed by the plugin
host contract and CFFI is fixed by the workflow requirement.

## Maintenance Rule

Any change to an installed third-party DLL, runtime, font, icon set, data set,
or independently licensed plugin must update the component manifest and its
licence evidence in the same pull request. A version-only edit is insufficient
when the upstream licence or notices changed.

Before a public release, review the generated table and SBOM against the
validated MSI contents and retain the legal bundle with the release artefacts.
