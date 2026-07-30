# C# Plugin Guide

## Scope

This guide covers C# add-ons for Fabulor and links to shared schema and troubleshooting rules.

Fabulor exposes only the current managed contract types documented below.
Add-ons compiled against former product-named types must update their source
and rebuild against the installed `Fabulor.PluginAbstractions.dll`; no silent
managed compatibility types are provided.

Read shared rules first:

1. [Simple Add-ons](simple-addons.md)
2. [Plugin Schema, Compatibility, and Troubleshooting](plugin-schema-and-troubleshooting.md)

## Simple C# Add-on

Preferred layout:

```text
%APPDATA%\Fabulor\addons\helper\
  helper.dll
  dependency.dll
```

Fabulor does not compile C# source. Create a .NET 8 class-library project whose assembly name matches the add-on folder:

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net8.0</TargetFramework>
    <AssemblyName>helper</AssemblyName>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>enable</Nullable>
  </PropertyGroup>

  <ItemGroup>
    <Reference Include="Fabulor.PluginAbstractions">
      <HintPath>C:\Program Files\Fabulor\Runtime\DotNet\Fabulor.PluginAbstractions.dll</HintPath>
      <Private>false</Private>
    </Reference>
  </ItemGroup>
</Project>
```

Build the project and copy `helper.dll` plus any private dependencies into the `helper` add-on folder. Do not copy `Fabulor.PluginAbstractions.dll`; Fabulor supplies its installed contract assembly. Restart Fabulor to load a new build.

The entry assembly must contain one concrete, constructible implementation of `IFabulorPlugin`:

```csharp
using Fabulor.Plugins;

public sealed class HelperPlugin : IFabulorPlugin
{
    public void Init(FabulorContext context)
    {
        context.Log("Helper initialised");
    }
}
```

## Advanced C# plugin.json

```json
{
  "id": "example.csharp.greeter",
  "name": "CSharp Greeter",
  "version": "1.0.0",
  "language": "csharp",
  "entrypoint": "GreeterPlugin.dll",
  "requires_api_version": 1,
  "dependencies": [],
  "capabilities": ["events.message", "session.read"],
  "description": "Minimal C# event-observer plugin.",
  "author": "Fabulor",
  "homepage": "https://github.com/Fabulor/fabulor"
}
```

## Minimal C# Plugin

```csharp
using Fabulor.Plugins;

public sealed class GreeterPlugin : IFabulorPlugin
{
    private FabulorContext? _context;
    private bool _reportedFirstMessage;

    public void Init(FabulorContext context)
    {
        _context = context;
        var user = context.GetUserInfo();
        context.Log($"Hello, {user.Nickname ?? "unknown"}. C# sample ready.");
        context.RegisterCallback("message", OnMessage);
    }

    private void OnMessage(FabulorEvent evt)
    {
        if (_reportedFirstMessage || _context is null)
            return;

        _reportedFirstMessage = true;
        var location = string.IsNullOrWhiteSpace(evt.Channel)
            ? "the active session"
            : evt.Channel;
        _context.Log($"C# sample observed its first incoming message event in {location}.");
    }
}
```

## Notes

1. Keep callback handlers lightweight to avoid blocking the main thread.
2. Use the compiled simple `addons\<name>\<name>.dll` layout for personal C# add-ons. The folder and assembly basenames must match.
3. Simple profile add-ons are trusted local code and do not declare capabilities. Advanced manifest plug-ins must declare every host operation they use; undeclared message, session-read, and callback operations are denied.
4. The managed contract assembly lives in `src\managed\Fabulor.PluginAbstractions` and currently defines `IFabulorPlugin`, `FabulorContext`, `FabulorEvent`, and `FabulorUserInfo`.
5. C# manifests load through the `src\managed\Fabulor.PluginHost` bridge, which calls `IFabulorPlugin.Init(...)` and routes callbacks back through the shared native registry.
6. The installer stages the managed bridge assemblies together with a bundled private `.NET` runtime under `Runtime\DotNet`, preserving `host\fxr` and `shared\Microsoft.NETCore.App`.
7. Normal manifest loading accepts only that installed executable-relative runtime and bridge root. Development overrides such as `FABULOR_DOTNET_ROOT`, `DOTNET_ROOT`, `FABULOR_CSHARP_BRIDGE_ROOT`, current-directory runtime roots, source-tree bridge outputs, and the machine-wide .NET installation require `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1`.
8. `FabulorContext.RegisterCallback(...)` can subscribe to generic events such as `message`, `server`, `print`, and `command`, as well as specific forms like `server:NOTICE`, `print:Channel Message`, or `command:SAY`. `message` represents an incoming IRC `PRIVMSG`; locally entered channel text uses the outgoing `command:SAY` event unless the server echoes it back.
9. `FabulorEvent` exposes richer payload accessors including `Channel`, `Network`, `Nick`, `Server`, `Time`, `Word1`-`Word4`, and `WordEol1`-`WordEol2`.
10. `FabulorContext.GetUserInfo()` returns the active session identity as a `FabulorUserInfo` with `Nickname`, `Channel`, `ServerName`, and `NetworkName`.
11. Maintained simple and manifest C# samples live under `samples\plugins\simple-csharp-greeter\` and `samples\plugins\example.csharp.greeter\`. The simple project's assembly name matches its folder and produces the exact profile entrypoint `simple-csharp-greeter.dll`. For the manifest sample, deploy `plugin.json`, `GreeterPlugin.dll`, and any plugin-owned dependencies together as direct children of one enabled manifest plugin folder. Build-tree paths such as `bin\Release\net8.0\GreeterPlugin.dll` are not valid manifest entrypoints.
12. Callback event names are limited to 128 UTF-8 bytes, generated handler names to 256 bytes, each plugin to 64 callbacks, and each event to 256 callbacks. Registering the same event/handler pair twice is rejected.
13. Manifest C# receives only the five shared operations represented by `FabulorContext`: log, send message, read user count, read user information, and register callback. The native boundary repeats capability and input validation for every privileged call.
