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
  "entrypoint": "GreeterPlugin.dll",
  "requires_api_version": "1",
  "dependencies": [],
  "capabilities": ["messages.write", "events.message"],
  "description": "Minimal C# greeting plugin.",
  "author": "Fabulor",
  "homepage": "https://github.com/Fabulor/fabulor"
}
```

## Minimal C# Plugin

```csharp
using System;

public sealed class GreeterPlugin : IZoiteChatPlugin
{
    public void Init(ZoiteChatContext ctx)
    {
        ctx.Log("C# plugin initialised");
        ctx.SendMessage("#zoitechat", "Hello from C# plugin");
        ctx.RegisterCallback("message", OnMessage);
    }

    private void OnMessage(dynamic evt)
    {
        // Keep callback handlers resilient and non-blocking.
    }
}
```

## Notes

1. Keep callback handlers lightweight to avoid blocking the main thread.
2. Keep manifest capabilities aligned with actual plugin behaviour.
3. The current host scaffold keeps `ZoiteChatAPI` as a compatibility alias for `FabulorAPI` while the repo finishes the broader rebrand.
4. C# manifests are validated and included in dependency analysis, but the runtime CLR loader is not wired into startup yet.
