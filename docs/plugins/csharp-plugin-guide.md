# C# Plugin Guide

## Scope

This guide covers the minimal structure for a Fabulor C# plugin and links to shared schema and troubleshooting rules.

Read shared rules first:

1. [Plugin Schema, Compatibility, and Troubleshooting](plugin-schema-and-troubleshooting.md)

## C# plugin.json

```json
{
  "id": "example.csharp.greeter",
  "name": "CSharp Greeter",
  "version": "1.0.0",
  "language": "csharp",
  "entrypoint": "bin\\Release\\net8.0\\GreeterPlugin.dll",
  "requires_api_version": "1",
  "dependencies": ["example.tcl.greeter"],
  "capabilities": ["messages.write", "events.message", "session.read"],
  "description": "Minimal C# greeting plugin.",
  "author": "Fabulor",
  "homepage": "https://github.com/Fabulor/fabulor"
}
```

## Minimal C# Plugin

```csharp
using Fabulor.Plugins;

public sealed class GreeterPlugin : IZoiteChatPlugin
{
    public void Init(ZoiteChatContext ctx)
    {
        var user = ctx.GetUserInfo();
        var target = string.IsNullOrWhiteSpace(user.Channel) ? "#fabulor" : user.Channel;
        ctx.Log("C# plugin initialised");
        ctx.SendMessage(target, $"Hello from C# plugin as {user.Nickname ?? "unknown"}");
        ctx.RegisterCallback("message", OnMessage);
    }

    private void OnMessage(ZoiteChatEvent evt)
    {
        // Keep callback handlers resilient and non-blocking.
    }
}
```

## Notes

1. Keep callback handlers lightweight to avoid blocking the main thread.
2. Keep manifest capabilities aligned with actual plugin behaviour.
3. The managed contract assembly lives in `src\managed\Fabulor.PluginAbstractions` and currently defines `IZoiteChatPlugin`, `ZoiteChatContext`, `ZoiteChatEvent`, and `ZoiteChatUserInfo`.
4. The current host scaffold keeps `ZoiteChatAPI` as a compatibility alias for `FabulorAPI` while the repo finishes the broader rebrand.
5. C# manifests now load through the `src\managed\Fabulor.PluginHost` bridge, which calls `IZoiteChatPlugin.Init(...)` and routes callbacks back through the shared native registry.
6. The installer now stages the managed bridge assemblies together with a bundled private `.NET` runtime root under `Runtime\DotNet`, including `hostfxr.dll` and the shared runtime payload.
7. `ZoiteChatContext.RegisterCallback(...)` can subscribe to generic events such as `message`, `server`, `print`, and `command`, as well as specific forms like `server:NOTICE`, `print:Channel Message`, or `command:SAY`.
8. `ZoiteChatEvent` now exposes richer payload accessors including `Channel`, `Network`, `Nick`, `Server`, `Time`, `Word1`-`Word4`, and `WordEol1`-`WordEol2`.
9. `ZoiteChatContext.GetUserInfo()` returns the active session identity as a `ZoiteChatUserInfo` with `Nickname`, `Channel`, `ServerName`, and `NetworkName`.
10. A maintained sample manifest C# plugin lives under `samples\plugins\example.csharp.greeter\`.
