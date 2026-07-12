# Tcl Plugin Guide

## Scope

This guide covers Tcl scripting for Fabulor and links to shared schema and troubleshooting rules.

Read shared rules first:

1. [Simple Add-ons](simple-addons.md)
2. [Plugin Schema, Compatibility, and Troubleshooting](plugin-schema-and-troubleshooting.md)

## Simple Tcl Add-on

Preferred layout:

```text
%APPDATA%\Fabulor\addons\aliases\aliases.tcl
```

Optional metadata:

```tcl
# Fabulor-Name: Personal aliases
# Fabulor-Version: 1.0
# Fabulor-Description: Adds local helper commands
```

Minimal script:

```tcl
proc init {} {
    zoitechat::print "Tcl add-on initialised"
}
```

## Advanced Tcl plugin.json

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
2. Use the simple `addons\<name>\<name>.tcl` layout for personal scripts and aliases.
3. Declare every host operation the plugin uses. Tcl commands return `TCL_ERROR` with a plugin-specific message when the corresponding manifest capability is absent.
4. The embedded Tcl host currently exposes `zoitechat::command`, `zoitechat::print`, `zoitechat::log`, `zoitechat::add_user_command`, `zoitechat::remove_user_command`, `zoitechat::getinfo`, `zoitechat::get_user_info`, `zoitechat::nickcmp`, `zoitechat::send_message`, `zoitechat::get_user_count`, and `zoitechat::register_callback`.
5. `zoitechat::getinfo` currently covers the safe session-backed values `away`, `channel`, `configdir`, `host`, `libdirfs`, `modes`, `network`, `nick`, `server`, `topic`, `version`, `xchatdir`, and `xchatdirfs`.
6. `zoitechat::register_callback` routes manifest callbacks through the shared host registry and can subscribe to generic events such as `message`, `server`, `print`, and `command`, as well as specific forms like `server:NOTICE`, `print:Channel Message`, or `command:SAY`.
7. Callback payload JSON now includes richer context such as `source`, `time`, `channel`, `network`, `nick`, `server`, `word1`-`word4`, and `word_eol1`-`word_eol2` where the underlying event provides them.
8. Use safe mode when diagnosing Tcl startup faults so third-party plugins stay disabled while core startup is verified.
9. `zoitechat::get_user_info` returns a Tcl key/value list covering `nick`, `channel`, `server`, and `network`.
10. A maintained sample manifest Tcl plugin lives under `samples\plugins\example.tcl.greeter\`.
11. `zoitechat::add_user_command name command` registers a runtime User Command alias for the current session. It does not write `commands.conf`; use the built-in User Commands editor for persistent aliases.
12. Manifest Tcl uses the installed `Runtime\Tcl` root by default. `FABULOR_TCL_RUNTIME_ROOT` and current-working-directory runtime roots are development-only and require `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1`.
