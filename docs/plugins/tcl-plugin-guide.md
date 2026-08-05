# Tcl Plugin Guide

## Scope

This guide covers Tcl scripting for Fabulor and links to shared schema and troubleshooting rules.

Fabulor exposes its Tcl API through the `fabulor::*` namespace. Add-ons written
for the former product namespace must be updated; Fabulor does not provide a
silent legacy alias. This keeps stale add-ons from appearing to work against an
API name that the product no longer owns.

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
proc hello {arguments} {
    fabulor::print "Hello $arguments"
}

proc init {} {
    fabulor::register_command HELLO hello
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
  "requires_api_version": 2,
  "dependencies": [],
  "capabilities": ["events.command", "events.message", "session.read"],
  "description": "Minimal Tcl event-observer plugin.",
  "author": "Fabulor",
  "homepage": "https://github.com/Fabulor/fabulor"
}
```

## Minimal Tcl Plugin

```tcl
set reportedFirstMessage 0

proc onMessage {eventData} {
    global reportedFirstMessage
    if {$reportedFirstMessage} {
        return
    }

    set reportedFirstMessage 1
    array set user [fabulor::get_user_info]
    set location "the active session"
    if {[info exists user(channel)] && $user(channel) ne ""} {
        set location $user(channel)
    }
    fabulor::log "Tcl sample observed its first incoming message event in $location."
}

proc onGreeterCommand {eventData} {
    fabulor::log "Tcl greeter command handled."
    return consume
}

proc init {} {
    array set user [fabulor::get_user_info]
    set nick "unknown"
    if {[info exists user(nick)] && $user(nick) ne ""} {
        set nick $user(nick)
    }
    fabulor::log "Hello, $nick. Tcl sample ready."
    fabulor::register_callback message onMessage
    fabulor::register_callback command:GREETER onGreeterCommand
}
```

## Notes

1. Keep callback handlers lightweight to avoid blocking the main thread.
2. Use the simple `addons\<name>\<name>.tcl` layout for personal scripts and aliases.
3. Declare every host operation the plugin uses. Tcl commands return `TCL_ERROR` with a plugin-specific message when the corresponding manifest capability is absent.
4. Manifest Tcl exposes only the shared surface: `fabulor::log`, `fabulor::send_message`, `fabulor::get_user_count`, `fabulor::get_user_info`, and `fabulor::register_callback`.
5. Trusted simple Tcl add-ons additionally receive `fabulor::command`, `fabulor::print`, `fabulor::add_user_command`, `fabulor::remove_user_command`, `fabulor::register_command`, `fabulor::getinfo`, and `fabulor::nickcmp`. These helpers are not available to manifest Tcl and do not expand the shared `FabulorAPI`.
6. `fabulor::getinfo` currently covers the simple-add-on values `away`, `channel`, `configdir`, `host`, `libdirfs`, `modes`, `network`, `nick`, `server`, `topic`, `version`, `xchatdir`, and `xchatdirfs`.
7. Simple Tcl add-ons use `fabulor::register_command name handler`. The handler receives the command's remaining text as one argument. Duplicate command names are rejected.
8. `fabulor::register_callback` is manifest-only. It routes callbacks through the shared host registry and can subscribe to generic events such as `message`, `server`, `print`, and `command`, as well as specific forms like `server:NOTICE`, `print:Channel Message`, or `command:SAY`. `message` represents an incoming IRC `PRIVMSG`; locally entered channel text uses `command:SAY` unless the server echoes it back.
9. Callback payload JSON now includes richer context such as `source`, `time`, `channel`, `network`, `nick`, `server`, `word1`-`word4`, and `word_eol1`-`word_eol2` where the underlying event provides them.
10. Use safe mode when diagnosing Tcl startup faults so third-party plugins stay disabled while core startup is verified.
11. `fabulor::get_user_info` returns a Tcl key/value list covering `nick`, `channel`, `server`, and `network`.
12. Maintained simple and manifest Tcl samples live under `samples\plugins\simple-tcl-greeter\` and `samples\plugins\example.tcl.greeter\`.
13. `fabulor::add_user_command name command` registers a runtime User Command alias for the current session. It does not write `commands.conf`; use the built-in User Commands editor for persistent aliases.
14. Manifest Tcl uses the installed `Runtime\Tcl` root by default. `FABULOR_TCL_RUNTIME_ROOT` and current-working-directory runtime roots are development-only and require `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1`.
15. Callback event names are limited to 128 UTF-8 bytes, Tcl handler names to 256 bytes, each plugin to 64 callbacks, and each event to 256 callbacks. Registering the same event/handler pair twice is rejected.
16. The bundled runtime contains the Tcl 8.6 engine, core library, encodings,
    timezone/message data, and the standard `platform`, `msgcat`, `http`, and
    `tcltest` modules. It does not include Tk, Tcl command shells, development
    libraries, or third-party package collections. Add-ons that need another
    Tcl package must distribute and load that dependency within their own
    trusted add-on directory.
17. API version 2 callbacks may return `consume` (or `1`) to prevent the
    command from reaching Fabulor after all registered plugin callbacks have
    run. An empty return, `continue`, or any other value keeps normal
    processing. Use consumption for plugin-owned commands such as
    `command:GREETER`. Return values from server, message, and print callbacks
    do not suppress Fabulor's IRC state processing.
18. Shared API commands invoked inside a callback use that event's originating
    network and channel session. A JOIN callback can therefore call
    `fabulor::send_message` without accidentally targeting a same-named channel
    on another network.
