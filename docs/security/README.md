# Security Documentation

## Current Policy

- [User security and privacy guidance](../user/security-and-privacy.md)
  describes the product boundary, local data, credentials, add-ons,
  diagnostics, and current update status for ordinary users.
- [Trusted configuration](trusted-config.md) records the supported security
  policy for advanced local configuration.
- [Signed update feed design](signed-update-feed-design.md) defines the trust,
  release, verification, and activation gates for any future in-client updater.
- [SignPath Foundation eligibility audit](signpath-foundation-eligibility-audit.md)
  records the code-signing application criteria, current evidence, gaps, and
  required remediation order.
- [SignPath artifact and metadata contract](signpath-artifact-metadata.md)
  defines the canonical Windows metadata and the exact Fabulor-owned files
  eligible for the initial signing configurations.
- [Code signing policy](../../CODE_SIGNING_POLICY.md) defines accountable team
  roles, review requirements, the signing allowlist, manual approval, and
  incident response.
- [Privacy policy](../../PRIVACY.md) defines Fabulor's user-directed network and
  local-data boundary.
- [Signed release disclosure](signed-release-disclosure.md) provides the
  required policy and attribution block for future signed release pages.
- Plugin schema, enablement, capability, and troubleshooting guidance lives
  under [`docs/plugins`](../plugins).

## Historical Evidence

Completed audits and incident analyses are retained under
[`archive`](archive/README.md). They preserve the state, tools, findings, and
remediation evidence from the recorded review rather than defining current
configuration policy.
