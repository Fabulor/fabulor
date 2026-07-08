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
  "dependencies": ["example.python.greeter"],
  "capabilities": ["messages.write", "events.message", "session.read"],
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
    array set user [zoitechat::get_user_info]
    set target "#fabulor"
    if {[info exists user(channel)] && $user(channel) ne ""} {
        set target $user(channel)
    }
    zoitechat::log "Tcl plugin initialised"
    zoitechat::send_message $target "Hello from Tcl plugin"
    zoitechat::register_callback message onMessage
}
```

## Notes

1. Keep callback handlers lightweight to avoid blocking the main thread.
2. Keep manifest capabilities aligned with actual plugin behaviour.
3. The embedded Tcl host currently exposes `zoitechat::command`, `zoitechat::print`, `zoitechat::log`, `zoitechat::getinfo`, `zoitechat::get_user_info`, `zoitechat::nickcmp`, `zoitechat::send_message`, `zoitechat::get_user_count`, and `zoitechat::register_callback`.
4. `zoitechat::getinfo` currently covers the safe session-backed values `away`, `channel`, `configdir`, `host`, `libdirfs`, `modes`, `network`, `nick`, `server`, `topic`, `version`, `xchatdir`, and `xchatdirfs`.
5. `zoitechat::register_callback` routes manifest callbacks through the shared host registry and can subscribe to generic events such as `message`, `server`, `print`, and `command`, as well as specific forms like `server:NOTICE`, `print:Channel Message`, or `command:SAY`.
6. Callback payload JSON now includes richer context such as `source`, `time`, `channel`, `network`, `nick`, `server`, `word1`-`word4`, and `word_eol1`-`word_eol2` where the underlying event provides them.
7. Use safe mode when diagnosing Tcl startup faults so third-party plugins stay disabled while core startup is verified.
8. `zoitechat::get_user_info` returns a Tcl key/value list covering `nick`, `channel`, `server`, and `network`.
9. A maintained sample manifest Tcl plugin lives under `samples\plugins\example.tcl.greeter\`.
