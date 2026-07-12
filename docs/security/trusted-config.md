# Trusted Configuration Security Notes

Fabulor treats its own profile/configuration files as trusted local user state.

This means a local user can deliberately configure advanced behaviours that would be unsafe if they came from chat traffic, downloaded plugin manifests, or silent migration/import code.

## Absolute Log Masks

The log filename preference accepts absolute paths by design. When the log mask is relative, logs are written under the profile `logs` directory. When the log mask is absolute, Fabulor writes to the expanded absolute path after normal filename sanitisation.

Security policy:

1. Absolute log masks are a trusted-config capability.
2. Normal chat traffic must not be able to change the log mask.
3. Future config import or migration code must either reject absolute log masks or require explicit user confirmation.
4. UI that edits the log mask should keep absolute paths visible to the user.

## Invalid TLS Certificates

Server entries can persist the choice to accept invalid TLS certificates. Command-line connection options also expose explicit SSL no-verify switches.

Security policy:

1. Certificate verification stays enabled by default.
2. Invalid-certificate acceptance is a trusted-config exception, not a silent fallback.
3. Defaults, migration code, and imported server lists must not enable invalid-certificate acceptance without explicit user intent.
4. UI labels and diagnostics should continue to describe this as insecure.

