# Tcl Plugin Guide

## Scope

This guide covers the minimal structure for a Fabulor Tcl plugin and links to shared schema and troubleshooting rules.

Read shared rules first:

1. [Plugin Schema, Compatibility, and Troubleshooting](plugin-schema-and-troubleshooting.md)

## Tcl plugin.json

```json
{
  "id": "example.tcl.greeter",
  "name": "Tcl Greeter",
  "version": "1.0.0",
  "language": "tcl",
  "entrypoint": "plugin.tcl",
  "requires_api_version": "1",
  "dependencies": [],
  "capabilities": ["messages.write", "events.message"],
  "description": "Minimal Tcl greeting plugin.",
  "author": "Fabulor",
  "homepage": "https://github.com/Fabulor/fabulor"
}
```

## Minimal Tcl Plugin

```tcl
proc onMessage {eventData} {
    # Keep callback handlers resilient and non-blocking.
    return
}

proc init {} {
    zoitechat::log "Tcl plugin initialised"
    zoitechat::send_message "#zoitechat" "Hello from Tcl plugin"
    zoitechat::register_callback message onMessage
}
```

## Notes

1. Keep callback handlers lightweight to avoid blocking the main thread.
2. Keep manifest capabilities aligned with actual plugin behaviour.
3. Use safe mode when diagnosing Tcl startup faults so third-party plugins stay disabled while core startup is verified.
4. Tcl manifests are validated and included in dependency analysis, but the Tcl runtime loader is not wired into startup yet.
