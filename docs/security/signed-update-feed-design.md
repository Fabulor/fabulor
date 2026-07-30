# Signed Update Feed Design

Status: design accepted; implementation and activation remain blocked.

Fabulor does not currently perform in-client update checks. The inherited
WinSparkle integration was removed because its externally controlled feed
offered ZoiteChat packages to Fabulor users. This document defines the security
and release boundary that must exist before a Fabulor-owned update check can be
restored.

## Scope

The first implementation is limited to:

- Windows 11 x64 installed builds;
- the stable release channel;
- checking for and describing an available update;
- downloading the full Fabulor bootstrapper after explicit user approval; and
- launching that bootstrapper only after all verification succeeds.

The updater will not patch files in place, silently install an update, update
plugins, or replace a running executable. Portable builds may report an
available release but must not install automatically.

## Security Goals

The update path must:

1. accept release metadata only when authorized by Fabulor update keys;
2. reject modified, truncated, mixed, expired, or rolled-back metadata;
3. bind the advertised version to an exact bootstrapper length and SHA-256
   digest;
4. reject a bootstrapper that is not Authenticode-signed by an authorized
   Fabulor publisher;
5. keep a failed or interrupted update from affecting the installed client;
6. preserve normal client operation when the feed is offline or invalid; and
7. avoid sending installation identifiers, network names, nicknames, or other
   profile data.

Availability is not guaranteed. A network, hosting, timestamp-key, or
certificate failure may prevent an update from being offered, but it must
never turn an unverified file into an accepted update.

## Threat Model

The design covers:

- compromise of the metadata or release hosting account;
- a malicious or compromised mirror;
- TLS interception or redirect manipulation;
- replay, rollback, freeze, and mix-and-match attacks;
- a truncated or oversized response;
- replacement of a downloaded file before launch;
- compromise of one offline metadata key;
- compromise of the online timestamp key; and
- accidental publication of mismatched release assets and metadata.

A local administrator and deliberate modification by the current user remain
outside the protection boundary. The updater still uses restrictive temporary
directories and atomic state writes to avoid creating unnecessary local attack
surface.

## Metadata Standard

Fabulor will use The Update Framework (TUF) specification version 1.0.33 with
consistent snapshots. A custom appcast, unsigned GitHub API response, release
tag, or checksum text file is not an update authorization.

The repository contains the standard top-level roles:

- `root`: update-key identities, thresholds, and supported algorithms;
- `targets`: exact release target length, SHA-256 digest, and Fabulor-specific
  target properties;
- `snapshot`: one coherent set of targets metadata;
- `timestamp`: short-lived freshness for the current snapshot.

The initial client embeds a reviewed `root.json`. Root rotation follows the TUF
sequential root-update process. The client must not trust a root fetched only
because it came from the configured HTTPS host.

The stable bootstrapper target uses an immutable path such as:

`stable/1.0.7/FabulorSetup-x64.exe`

Its signed custom properties contain:

- product identifier `org.fabulor.windows`;
- semantic version;
- Windows installer product version;
- channel `stable`;
- architecture `x64`;
- minimum supported Windows build;
- release-notes HTTPS URL; and
- whether the release is a normal or security update; and
- the accepted Authenticode publisher SubjectPublicKeyInfo SHA-256 digest.

Unknown required fields, duplicate JSON keys, invalid UTF-8, non-canonical
metadata, unsupported algorithms, and values outside defined bounds are
rejected.

## Key Policy

The activation baseline is:

| Role | Threshold | Storage |
| --- | ---: | --- |
| Root | 2 of 3 | Separate offline devices; never GitHub Actions |
| Targets | 2 of 3 | Separate release-signing devices |
| Snapshot | 1 of 1 | Release-signing environment |
| Timestamp | 1 of 1 | Restricted online publishing service |

The root and targets private keys must not be repository secrets, workflow
secrets, developer workstation files, or release assets. Recovery material is
kept offline and tested before activation.

Root metadata authorizes more than one future public key before a planned
rotation. A compromised or lost key is revoked through a threshold-signed root
update. Emergency response can expire or replace targets without publishing a
new client.

The Authenticode signing key must be hardware-backed or held by a managed
code-signing service. Threshold-signed targets metadata pins the SHA-256
SubjectPublicKeyInfo digest of the accepted Fabulor publisher key for each
bootstrapper. A normal Windows trust-chain success from an unrelated publisher
is insufficient.

## Feed And Transport Boundary

Metadata is published from a Fabulor-controlled, static HTTPS origin. Mirrors
are optional because TUF signatures, versions, lengths, and hashes establish
content authenticity independently of a mirror. HTTPS remains mandatory for
privacy and availability protection.

The client:

- uses the Windows proxy and certificate policy;
- has no certificate-validation bypass;
- permits only HTTPS redirects, with a small fixed redirect limit;
- applies connection, response, and total-operation timeouts;
- bounds root, timestamp, snapshot, and targets response sizes;
- bounds the bootstrapper to a documented maximum size;
- does not send cookies, credentials, referrers, or profile data; and
- identifies only the Fabulor version, architecture, and update protocol
  version in its user agent.

Feed failure is reported as an update-check failure, not as "up to date."

## Verification Order

The implementation must perform these steps in order:

1. Load the embedded or last trusted root and persistent anti-rollback state.
2. Apply sequential, threshold-valid root updates.
3. Fetch and verify timestamp, snapshot, and targets metadata according to TUF.
4. Reject expired metadata and metadata versions below the highest trusted
   versions.
5. Select only the stable Windows x64 Fabulor target.
6. Compare versions using one strict semantic-version implementation.
7. Ask the user before downloading.
8. Download into a newly created, current-user-only temporary directory.
9. Verify the exact signed target length and SHA-256 digest.
10. Call `WinVerifyTrust` with the Authenticode policy and require a zero
    success result.
11. Build and verify the signer certificate chain with revocation checking.
12. Extract the signer public key and match its SPKI digest against the value
    in threshold-authorized targets metadata.
13. Recheck the file identity, length, digest, and signature immediately before
    launch.
14. Launch the verified bootstrapper only after a second explicit confirmation.

Any failure deletes the candidate where possible, records a bounded diagnostic,
and leaves the running installation untouched.

## Rollback And Freeze Protection

The client persists:

- highest trusted root, timestamp, snapshot, and targets versions;
- highest accepted stable product version;
- the last trusted metadata time; and
- hashes of the active trusted metadata.

State is written atomically under the per-user Fabulor update-state directory.
The effective time for expiry checks cannot move behind the last trusted time.
The updater never offers a version below the installed or highest accepted
stable version. Manual installation of an older package remains a separate,
explicit user action outside the updater.

Timestamp metadata expires within 24 hours. Snapshot and targets metadata use
longer but bounded expirations, and root metadata is rotated well before its
expiry. Publishing refreshes timestamp last so clients cannot observe a
partially promoted metadata set.

## Release Promotion

An update-capable release follows this order:

1. Build and validate the MSI and bootstrapper from a protected release tag.
2. Confirm the bootstrapper embeds the byte-identical validated MSI.
3. Authenticode-sign and timestamp the bootstrapper.
4. Re-run bundle validation and calculate final length and SHA-256 values.
5. Create a draft release containing immutable versioned assets and hashes.
6. Generate unsigned TUF target input from those final assets.
7. Obtain the required offline targets signatures.
8. Generate and sign coherent snapshot metadata.
9. Publish release assets and versioned metadata.
10. Publish the short-lived timestamp metadata last.
11. Independently verify the public repository with a clean updater verifier.
12. Promote the draft release only after verification succeeds.

GitHub artifact attestations and SBOM attestations should be added when the
repository and account plan support them. They provide useful build provenance
but do not replace TUF authorization or Authenticode verification.

## User Experience

The initial UI is deliberately small:

- `Help > Check for Updates` performs a manual check;
- no automatic download or installation occurs;
- the result shows version, release date, security-update status, and a release
  notes link derived from signed metadata;
- download and launch are separate explicit actions;
- verification failure never exposes a Run or Install action; and
- dismissing the dialog changes no update state.

Scheduled notification checks may be considered only after the manual path has
passed installed acceptance testing. They must be rate-limited and contain
jitter; they still must not download automatically.

## Implementation Passes

1. **Release trust infrastructure**
   - acquire the Authenticode identity;
   - provision root, targets, snapshot, and timestamp keys;
   - select the metadata origin; and
   - document key backup, rotation, revocation, and incident response.
2. **Bounded TUF verifier**
   - select a maintained implementation or perform a separate cryptographic
     design review before writing one;
   - add malformed, expiry, rollback, mix-and-match, and key-rotation tests;
   - fuzz metadata parsing; and
   - keep all network and GTK dependencies outside the verifier.
3. **Artifact verifier and downloader**
   - add bounded HTTPS download;
   - enforce target length and digest;
   - add Authenticode chain, revocation, and SPKI-pin verification; and
   - exercise replacement and interruption cases.
4. **Manual UI**
   - restore the Help action;
   - present signed release information;
   - keep download and launch user-driven; and
   - add accessible keyboard and screen-reader behavior.
5. **Release and installed acceptance**
   - verify clean, upgrade, repair, cancellation, offline, proxy, invalid-feed,
     expired-feed, rollback, and revoked-certificate behavior;
   - validate a key rotation; and
   - publish hashes and independent verification instructions.

## Activation Gate

The in-client action remains absent until all of these are true:

- an exact TUF client implementation and cryptographic backend are selected;
- reviewed root metadata is embedded;
- threshold root and targets keys are provisioned and recoverable;
- the Authenticode publisher identity and SPKI pins exist;
- the static metadata origin is controlled by Fabulor;
- release promotion produces immutable, signed, independently verified output;
- rollback state and failure behavior pass automated tests; and
- an installed release-candidate update succeeds without bypasses.

## References

- [The Update Framework specification](https://theupdateframework.org/spec/)
- [Microsoft WinVerifyTrust documentation](https://learn.microsoft.com/en-us/windows/win32/api/wintrust/nf-wintrust-winverifytrust)
- [GitHub artifact attestation documentation](https://docs.github.com/en/actions/how-tos/secure-your-work/use-artifact-attestations/use-artifact-attestations)
