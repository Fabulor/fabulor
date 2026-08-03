# SignPath Foundation Eligibility Audit

Audit date: 2026-08-03  
Repository state reviewed: `ca2b422f80e12917f11d03e160def342efafb86f`  
MFA evidence updated: 2026-08-04

Scope: SignPath Foundation eligibility and application readiness for the
Fabulor Windows MSI and bootstrapper.

This is an engineering readiness assessment against the published SignPath
Foundation conditions. It is not legal advice and does not predict SignPath
Foundation's final eligibility decision.

## Outcome

Fabulor appears suitable in purpose and licensing for a SignPath Foundation
application, but it is **not ready to apply yet**.

No inherent disqualifier was found. Fabulor is an actively maintained GPLv3
IRC client, not a security-circumvention or vulnerability-scanning tool. It
has released Windows installers, substantial user documentation, user-directed
network activity, explicit installer choices, and normal uninstall support.

The remaining gaps are remediable release-engineering and governance work:

1. Make the source repository public before applying.
2. Retain the verified organization-wide secure multi-factor authentication
   posture and require it for every future GitHub or SignPath participant.
3. Protect the release branch and publish clear author, reviewer, and signing
   approver responsibilities.
4. Publish the prepared Code signing policy, required SignPath attribution,
   named roles, and privacy statement when the repository becomes public.
5. Produce release candidates through a verifiable hosted build and promote
   those exact artifacts to SignPath with manual signing approval.
6. Normalize signed-file product metadata.
7. The bundled-component licence inventory and release SBOM are now complete;
   retain their validation as a release gate.

## Authority

The assessment uses the current official:

- [SignPath Foundation conditions for Open Source projects](https://signpath.org/terms.html)
- [SignPath Foundation application page](https://signpath.org/apply.html)

The conditions page currently labels its code of conduct as a draft. The
project should re-check the published terms immediately before applying.

## Eligibility Matrix

| Requirement | Status | Fabulor evidence and remaining action |
| --- | --- | --- |
| No malware or potentially unwanted programs | Provisional pass | Repository scanning, CodeQL, Gitleaks, dependency review, and targeted high-risk review are recorded in the security archive. Repeat scans against the final public release commit and artifact. |
| OSI-approved licence without commercial dual licensing | Pass for project code | Fabulor is GPLv3 and GitHub identifies the repository as GPL-3.0. No commercial dual-licensing scheme was found. The installed third-party inventory and evidence bundle are generated and packaged with each Windows build. |
| No proprietary components, except qualifying system libraries | Provisional pass | The audited payload is assembled from Fabulor and identified open-source runtimes and libraries, plus Microsoft's redistributed .NET runtime under the Microsoft .NET Library terms. Confirm SignPath accepts that runtime under its system-library allowance before applying. |
| Actively maintained | Pass | The repository has sustained development, reviews, release candidates, security remediation, documentation, and installed-client testing. |
| Already released in the form to sign | Pass | `v1.0.6-rc.4` publishes the Windows bootstrapper form intended for signing. The final candidate must be rebuilt through the protected release process. |
| Functionality documented on the download page or app entry | Pass | The repository README, release entries, and `docs/user` manual describe the product, installation, connections, commands, preferences, themes, add-ons, privacy, and troubleshooting. |
| Sign only the team's own project and binaries | Provisional pass | Fabulor owns its source and build scripts. Upstream OSS DLLs are included as unsigned installer payload, which the policy permits. The SignPath configuration must select only Fabulor-owned binaries and packages for signing. |
| No hacking tools | Pass | Fabulor is an IRC client. The optional Exec add-on is a bounded, user-invoked local command facility; it is not designed to identify vulnerabilities, exploit systems, or circumvent security measures. Development-only scanners are outside the shipped product. |
| Respect user privacy and security | Provisional pass | Network traffic is initiated by the user or configured IRC connections; the retired updater performs no background update traffic. Current user guidance documents local data and credentials. Publish the required concise privacy statement on the public project home page. |
| Announce system changes | Pass | Setup presents install scope and optional features. Protocol registration, shortcuts, runtime features, and other system integration are installer-owned and removable. |
| Provide uninstallation | Pass | WiX provides uninstall, modify, repair, and upgrade behavior; these paths have installed-client acceptance evidence. Final signed-artifact uninstall testing remains required. |
| MFA for all team members | Pass for GitHub | The GitHub organization enforces two-factor authentication with secure methods only. On 2026-08-04, GitHub's `2fa_disabled` member filter returned no accounts and all four current members were confirmed. Require MFA on every future SignPath account before applying. |
| Clear authors, reviewers, and signing approvers | Prepared | `CODE_SIGNING_POLICY.md` names the current author, reviewer, and signing approver, distinguishes Triage-only testers, and makes signing approval a separate manual decision. The assignments become public with the repository. |
| Public Code signing policy and attribution | Prepared | The root Code signing policy and README contain the required heading and SignPath attribution. A reusable signed-release disclosure is ready. Verify the public links and use that disclosure on each signed release page after activation. |
| Privacy policy or prescribed no-transfer statement | Prepared | `PRIVACY.md` contains the prescribed no-transfer statement and identifies user-directed IRC, proxy, bouncer, browser, add-on, GitHub, and future signing-service boundaries. It becomes public with the repository. |
| Consistent signed-file metadata | Pass | Product, company/publisher, descriptions, and version values now follow the canonical contract in `installer/Directory.Build.props`. CI validates the built application, GTK4 frontend, bootstrapper application, MSI, and bundle. `signpath-artifact-metadata.md` restricts the future SignPath configurations to the two Fabulor-owned application files, MSI, and bootstrapper and explicitly excludes third-party payloads. |
| Verifiable source-to-binary build | Blocked | GitHub Actions builds and validates the MSI and bootstrapper, but the release process does not yet promote those exact artifacts. `v1.0.6-rc.4` was published before the matching workflow run completed, so its provenance cannot be demonstrated from the current automation. |
| Manual approval for every signing request | Gap | There is no SignPath submission environment or manual approval gate yet. Add a protected GitHub environment and a SignPath policy with an explicit approver. |

## Repository And Governance Evidence

At audit time:

- `Fabulor/fabulor` is private, with `main` as its default branch.
- GitHub recognizes the repository licence as GPL-3.0.
- the Fabulor organization enforces two-factor authentication and the
  organization owner has selected secure methods only;
- GitHub's `2fa_disabled` member filter returned no accounts on 2026-08-04;
- all four current organization members have completed MFA, with one
  administrator and three Triage-only testers at repository level;
- the private repository's current GitHub plan does not expose branch
  protection for `main`; GitHub reports that the repository must become public
  or the plan must be upgraded.
- release candidates through `v1.0.6-rc.4` exist as published prereleases.

The repository may remain private while remediation is prepared. Public
visibility and branch protection should be enabled together before the
application so the published governance state is internally consistent. MFA
enforcement is already enabled; member compliance must be confirmed before
roles are published.

## Build And Release Provenance

`.github/workflows/windows-build.yml` builds the Windows application, optional
plugins, MSI, and bootstrapper, validates the staged and installed payload
contracts, and uploads `Fabulor.msi`, `FabulorSetup.exe`, and the legal bundle
as workflow artifacts.

The workflow already requests OIDC and attestation permissions, but it does
not generate an artifact attestation. It also does not publish a GitHub
release or submit an artifact to SignPath.

For `v1.0.6-rc.4`:

- the release asset is `FabulorSetup-v1.0.6-rc4.exe`;
- its recorded SHA-256 is
  `bed56883d0fab42db0005bd38eb5698d1710569558b8615c990dbf334819977c`;
- the asset was created at `2026-08-02T05:24:56Z`; and
- the matching successful Windows build for the tagged commit completed at
  `2026-08-02T05:44:28Z`.

The release asset therefore was not demonstrably promoted from that completed
workflow run. The final release pipeline must instead:

1. build from a protected release tag or approved workflow dispatch;
2. preserve the source commit, dependency inputs, and artifact hashes;
3. attest the unsigned artifacts;
4. submit the exact validated artifact to SignPath;
5. require manual signing approval;
6. verify Authenticode and RFC 3161 timestamps; and
7. publish only the verified signed outputs and their checksums.

## Bundled Component Evidence

The release inventory is implemented in `third-party/components.json` and is
documented in `docs/legal/bundled-components-and-sbom.md`. It covers 41
redistributed components and 41 preserved licence or notice files, including
GTK4, Python, Tcl, .NET, OpenSSL, Enchant, CFFI, certificate data, fonts,
icons, ISO data, the separately licensed native plugins, and the WiX Burn
engine embedded in the bootstrapper.

The Windows workflow generates a deterministic CycloneDX 1.6 SBOM and
human-readable third-party notice index, verifies all declared evidence, and
packages the complete legal bundle beneath `share/doc/fabulor/third-party`.
The same bundle is retained as a separate workflow artefact. Fixed external
archives are checksum-controlled, while inherited data without a trustworthy
upstream release number is explicitly identified as a repository snapshot.

This closes the packaging and traceability gap. It does not replace a final
licence review, and SignPath should be asked to confirm that the redistributed
.NET runtime falls within its permitted system-library exception.

## Required Remediation Order

### 1. Repository Preparation

- retain and review the generated bundled-component, licence, and SBOM bundle;
- retain the normalized product metadata and SignPath artifact restriction
  contract;
- add final malware, dependency, and artifact scanning;
- retain the generated SBOM and add artifact attestations; and
- retain the prepared Code signing policy, privacy statement, ownership map,
  and signed-release disclosure;

### 2. Public Governance

- make `Fabulor/fabulor` public;
- confirm every participating organization and SignPath account complies with
  the enforced secure-MFA policy;
- protect `main` and release tags;
- require review for changes to source, build, installer, and workflow files;
  and
- publish the prepared policies and named authors, reviewers, and signing
  approvers, then verify every public policy link.

### 3. SignPath Integration

- configure a SignPath project and artifact configuration;
- restrict signing to Fabulor-owned outputs;
- connect SignPath to the verifiable GitHub Actions build;
- require a protected manual approval for every signing request; and
- place the required attribution on the home and release pages.

### 4. Signed Release Acceptance

- verify signatures, certificate chain, timestamps, hashes, and metadata;
- test clean install, upgrade, repair, uninstall, protocol registration, and
  optional features using the signed bootstrapper and MSI;
- publish release notes, checksums, provenance, and SBOM; and
- re-check the SignPath terms immediately before submitting the application.

## Decision

Do not apply yet. Complete repository preparation first, then make the
repository public and enable the required governance controls. Once the
release workflow can prove that the exact reviewed source produced the exact
artifact submitted for manual signing, Fabulor should be in a credible
position to apply.
