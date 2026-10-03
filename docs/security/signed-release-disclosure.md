# Signed Release Disclosure

Status as of 2026-10-04: historical template for the retired SignPath proposal.
Do not use this attribution for current releases. The final `1.0.6` WiX
candidate is unsigned, and planned WinGet distribution does not sign it. A
future signing route requires a new disclosure reflecting its actual provider.

Use this disclosure on every release page containing artifacts signed through
the Fabulor SignPath project. Do not use it for an unsigned release candidate.
Before publishing, replace every `SOURCE_COMMIT` placeholder with the full
immutable commit SHA used to build and sign the release.

## Code Signing Policy

Free code signing provided by SignPath.io, certificate by SignPath Foundation.

- [Fabulor Code signing policy](https://github.com/Fabulor/fabulor/blob/SOURCE_COMMIT/CODE_SIGNING_POLICY.md)
- [Fabulor privacy policy](https://github.com/Fabulor/fabulor/blob/SOURCE_COMMIT/PRIVACY.md)
- [Fabulor security policy](https://github.com/Fabulor/fabulor/blob/SOURCE_COMMIT/SECURITY.md)

The release notes must also identify the source commit and workflow run, list
the signed installer filenames and SHA-256 hashes, and link the release SBOM
and provenance evidence.
