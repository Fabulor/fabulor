# Code Signing Policy

This policy defines who may authorize Fabulor code signing, which artifacts
may be signed, and the evidence required before a signed Windows release is
published.

Free code signing provided by SignPath.io, certificate by SignPath Foundation.

Fabulor has not yet activated SignPath signing. Until that activation is
announced on a release page, published release candidates must be treated as
unsigned even when their hashes are provided.

## Team Roles

| Role | Current member | Responsibility |
| --- | --- | --- |
| Project lead and maintainer | [Barry Suridge (`@Alcheri`)](https://github.com/Alcheri) | Maintains the repository, appoints trusted role holders, and owns the release process. |
| Authors and committers | [Barry Suridge (`@Alcheri`)](https://github.com/Alcheri) | May author project changes and maintain Fabulor-owned source and build scripts. |
| Reviewers | [Barry Suridge (`@Alcheri`)](https://github.com/Alcheri) | Reviews contributions from people without commit access and verifies release-sensitive changes under the repository rules. |
| Signing approver | [Barry Suridge (`@Alcheri`)](https://github.com/Alcheri) | Performs the distinct manual approval for each signing request after all required evidence is available. |

Repository members with Triage access are testers and issue triagers. They are
not authors, reviewers, or signing approvers unless this policy is changed to
name them explicitly.

Every person participating in source access, review, release preparation, or
signing must use a secure multi-factor authentication method for GitHub and
SignPath. Role changes require repository-owner approval and a reviewed update
to this policy.

## Review Rules

- Changes from people without commit access require approval from a named
  reviewer before merge.
- Branch protection defines the minimum approval count. An author cannot use
  their own review to satisfy a required approval.
- Changes to source code, installer definitions, build or release workflows,
  dependency declarations, security controls, signing configuration, or this
  policy receive explicit release-boundary review.
- Required automated checks must pass on the exact commit selected for release.
- A signing approval is a separate manual decision. It is never inferred from
  merge permission, a passing build, a tag, or publication of a draft release.

## Signing Scope

The initial signing scope is restricted to Fabulor-owned artifacts:

- `fabulor.exe`;
- `fabulor-gtk4-frontend.dll`;
- `Fabulor.msi`; and
- `FabulorSetup.exe`.

The exact paths, metadata restrictions, nesting order, and exclusions are
defined in the
[SignPath artifact and metadata contract](docs/security/signpath-artifact-metadata.md).
Third-party runtime and library files bundled by Fabulor are not eligible for
the Fabulor signing identity.

## Approval Evidence

Before approving a signing request, the signing approver must verify that:

1. the request originates from the protected Fabulor repository and an
   approved release ref;
2. the reviewed commit, workflow run, unsigned artifacts, hashes, SBOM, and
   provenance evidence identify the same build;
3. all required tests, security checks, legal-inventory checks, and metadata
   checks passed;
4. the SignPath artifact configuration matched only the allowlisted files;
5. the release notes identify the version, supported Windows platform, known
   limitations, privacy policy, and this Code signing policy; and
6. no unresolved security or release-blocking defect remains.

After signing, the release process must verify the Authenticode chain, signed
file identity, RFC 3161 timestamp, package integrity, and published checksums.
Only those verified outputs may be promoted to a public release.

## Compromise Or Policy Violation

A suspected signing-policy violation, unauthorized artifact, compromised
account, or incorrect signature stops release promotion immediately. The
maintainer must preserve the evidence, notify SignPath when its service or
certificate may be affected, investigate the source and build boundary, and
request revocation when appropriate.

Security reports must follow [SECURITY.md](SECURITY.md). Concerns that a file
signed with a SignPath Foundation certificate violates SignPath's policy may
also be reported to `support@signpath.io` with concise supporting evidence.

## Related Policies

- [Privacy policy](PRIVACY.md)
- [Security policy](SECURITY.md)
- [Bundled components, licences, and SBOM](docs/legal/bundled-components-and-sbom.md)
- [SignPath Foundation eligibility audit](docs/security/signpath-foundation-eligibility-audit.md)
