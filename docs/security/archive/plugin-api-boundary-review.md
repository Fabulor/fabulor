# Manifest Plugin API Boundary Review

Date completed: 2026-07-31

## Scope

This review compared the actual C#, isolated Python, and Tcl manifest surfaces
with `FabulorAPI`, the callback registry, the documented capability schema, and
the maintained samples. It also decided whether additional shared helpers are
required before changing the manifest-host enablement policy.

## Trust Model

Manifest plugins are trusted local code running with the user's operating
system privileges. Python, Tcl, and managed code can use the language and OS
facilities available to the Fabulor process. The capability model is therefore
a cooperative Fabulor host boundary, not a process sandbox.

The host remains disabled by default and requires explicit trusted-code opt-in.
Completing this review does not justify enabling downloaded or unreviewed code
automatically.

## Supported Shared Surface

API version 1 remains deliberately compact:

| Operation | Capability |
| --- | --- |
| Log plugin text | none |
| Send one message | `messages.write` |
| Read active user count | `session.read` |
| Read active user/session identity | `session.read` |
| Register message, server, print, or command callbacks | matching `events.*` capability |

C# exposes these operations through `FabulorContext`. Isolated Python exports
only the corresponding five functions in its synthetic `fabulor` module.
Manifest Tcl now registers only the same five commands.

Trusted simple add-ons remain a separate boundary. In particular, simple Tcl
retains its command, UI-printing, alias, session-info, nickname-comparison, and
direct command-registration helpers. Those helpers are intentionally absent
from manifest Tcl.

## Findings And Resolution

1. **Manifest Tcl exposed trusted simple-add-on helpers.** The same interpreter
   registration function installed command execution, UI printing, alias
   management, extended session information, and nickname comparison for both
   modes. Registration is now conditional: manifest Tcl receives only the
   shared surface and callback registration; simple Tcl retains its trusted
   helper set.
2. **The schema accepted unreachable capabilities.** Command, preference,
   direct UI, timer, and unload capability names did not describe operations
   available across the supported manifest hosts. They are now rejected as
   unknown instead of implying an unsupported grant.
3. **Language checks needed a common final boundary.** Message target/text and
   log limits are now enforced by the native API as well as language-specific
   wrappers. Targets reject whitespace and control characters; text rejects CR
   and LF, preventing one message operation from being interpreted as another
   command or protocol line.
4. **Isolated Python operations already have defence in depth.** The isolated
   runtime validates capabilities before recording operations, and the trusted
   main interpreter validates the owning manifest again before applying them.
   A regression test now proves a forged isolated send operation is denied.
5. **C# privileged callbacks already carry plugin identity.** Native message,
   session, and callback delegates receive the manifest id and independently
   check the catalog before invoking the shared API.

## Helper Decision

No additional shared helpers are approved for API version 1. The maintained
manifest samples require only the existing surface, and speculative helpers
would enlarge the capability and lifetime model without a concrete use case.

A future helper requires:

1. a demonstrated cross-language add-on requirement;
2. an additive API/version and compatibility decision;
3. a documented capability mapping where the operation is privileged;
4. equivalent C#, Python, and Tcl enforcement; and
5. native, language-host, packaging, and installed-client tests.

The absence of additional helpers is not the reason the host remains
off-by-default. Explicit opt-in remains the correct policy because manifest
plugins are trusted code rather than sandboxed extensions.

## Validation

The completed gate passed:

- repository contracts: 35/35;
- GTK4 and installer profile contracts: 95/95;
- Python capability enforcement: 11/11;
- Python subinterpreter isolation: 7/7;
- managed host and both maintained C# samples: zero warnings and errors;
- Release x64 solution: zero warnings and errors;
- native manifest, path, theme, and service-message suite: 37/37;
- native extension ownership: eight modules, one data file, and fourteen
  owned import edges validated;
- production MSI: 2,858 installed files, no legacy GTK files, and all 1,431
  runtime-manifest content hashes verified; and
- production bootstrapper: version 1.0.6, one chain package, and byte-for-byte
  embedded MSI identity verified.
