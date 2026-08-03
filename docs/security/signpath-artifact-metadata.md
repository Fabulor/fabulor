# SignPath Artifact And Metadata Contract

This document defines the metadata and signing boundary for Fabulor's Windows
release artifacts. It is the source contract for the future SignPath artifact
configurations; provisioning those configurations remains a separate release
gate.

## Canonical Metadata

| Field | Value |
| --- | --- |
| Product | `Fabulor` |
| Publisher and company | `Fabulor` |
| Application description | `Fabulor IRC Client` |
| GTK4 frontend description | `Fabulor GTK4 Frontend` |
| Installer description | `Fabulor Setup` |
| Bootstrapper application description | `Fabulor Windows Installer` |
| Release version | `1.0.6` |
| Numeric binary version | `1.0.6.0` |

`installer/Directory.Build.props` is the release metadata source of truth.
Native application resources, managed installer metadata, the MSI, and the
Burn bundle derive their values from that contract. The Windows workflow runs
`tools/release/validate_product_metadata.ps1` against the built files and also
checks the MSI and Burn registration metadata.

## Signing Allowlist

The initial SignPath configurations must sign only these Fabulor-owned
artifacts:

| Artifact | Required restrictions |
| --- | --- |
| `fabulor.exe` inside `Fabulor.msi` | Product `Fabulor`; company `Fabulor`; original filename `fabulor.exe`; application description and exact release version from this contract. |
| `fabulor-gtk4-frontend.dll` inside `Fabulor.msi` | Product `Fabulor`; company `Fabulor`; original filename `fabulor-gtk4-frontend.dll`; frontend description and exact release version from this contract. |
| `Fabulor.msi` | Subject/product `Fabulor`; author/publisher `Fabulor`; exact release version. Sign after the two allowlisted nested PE files. |
| `FabulorSetup.exe` | Product and description `Fabulor Setup`; company `Fabulor`; original filename `FabulorSetup.exe`; exact release version. |

The artifact configurations must use exact paths and metadata restrictions,
with one expected match for each file. They must not use an unrestricted
`*.exe`, `*.dll`, or recursive file-set rule. SignPath supports both PE/MSI
metadata restrictions and deep signing of selected files inside an MSI; the
configuration should use those controls rather than relying on filename
extensions alone.

## Explicit Exclusions

The Fabulor certificate must not be applied to:

- files beneath `Runtime/`, `python/`, `plugins/`, or other third-party payload
  directories;
- redistributed .NET, GTK4, Python, Tcl, OpenSSL, Enchant, or other upstream
  binaries;
- the embedded bootstrapper application or managed plugin-host assemblies in
  the initial signing scope;
- an artifact whose metadata or path does not match this contract; or
- any additional executable discovered by a wildcard.

An excluded file may be added only after its ownership and licence are
reviewed, its metadata is normalized, and this allowlist and the SignPath
configuration are changed through pull-request review.

## Packaging Order

The signing workflow must preserve inside-out package integrity:

1. validate the unsigned build and its provenance;
2. deep-sign the two allowlisted application files and then `Fabulor.msi`;
3. place that exact signed MSI in the final Burn bundle;
4. sign `FabulorSetup.exe`; and
5. verify Authenticode chains, RFC 3161 timestamps, metadata, package identity,
   hashes, and installed behavior before release.

SignPath documents the relevant artifact configuration syntax in its
[artifact configuration reference](https://docs.signpath.io/artifact-configuration/reference)
and [nested-signing examples](https://docs.signpath.io/artifact-configuration/examples).
