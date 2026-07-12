using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using Fabulor.Plugins;

namespace Fabulor.PluginHost;

internal sealed class ManagedPluginState
{
    public ManagedPluginState(string pluginId, PluginLoadContext loadContext, IZoiteChatPlugin plugin)
    {
        PluginId = pluginId;
        LoadContext = loadContext;
        Plugin = plugin;
    }

    public string PluginId { get; }

    public PluginLoadContext LoadContext { get; }

    public IZoiteChatPlugin Plugin { get; }

    public Dictionary<string, ZoiteChatEventHandler> Callbacks { get; } = new(StringComparer.Ordinal);
}

internal sealed class PluginLoadContext : AssemblyLoadContext
{
    private readonly AssemblyDependencyResolver _resolver;

    public PluginLoadContext(string pluginPath)
        : base($"Fabulor.PluginHost:{Path.GetFileNameWithoutExtension(pluginPath)}", isCollectible: true)
    {
        _resolver = new AssemblyDependencyResolver(pluginPath);
    }

    protected override Assembly? Load(AssemblyName assemblyName)
    {
        if (assemblyName.Name == typeof(IZoiteChatPlugin).Assembly.GetName().Name)
        {
            return typeof(IZoiteChatPlugin).Assembly;
        }

        var assemblyPath = _resolver.ResolveAssemblyToPath(assemblyName);
        return assemblyPath is null ? null : LoadFromAssemblyPath(assemblyPath);
    }
}

public static class NativeExports
{
    [StructLayout(LayoutKind.Sequential)]
    private struct NativeHostApi
    {
        public IntPtr Log;
        public IntPtr SendMessage;
        public IntPtr GetUserCount;
        public IntPtr GetUserInfo;
        public IntPtr RegisterCallback;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct NativeUserInfo
    {
        public IntPtr Nickname;
        public IntPtr Channel;
        public IntPtr ServerName;
        public IntPtr NetworkName;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct LoadPluginRequest
    {
        public IntPtr PluginId;
        public IntPtr AssemblyPath;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct DispatchCallbackRequest
    {
        public IntPtr PluginId;
        public IntPtr HandlerName;
        public IntPtr EventName;
        public IntPtr PayloadJson;
    }

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate void NativeLogDelegate(IntPtr textUtf8);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate int NativeSendMessageDelegate(IntPtr pluginIdUtf8, IntPtr targetUtf8, IntPtr textUtf8);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate uint NativeGetUserCountDelegate(IntPtr pluginIdUtf8);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate int NativeGetUserInfoDelegate(IntPtr pluginIdUtf8, out NativeUserInfo userInfo);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate int NativeRegisterCallbackDelegate(IntPtr pluginIdUtf8, IntPtr eventNameUtf8, IntPtr handlerNameUtf8);

    private static readonly Dictionary<string, ManagedPluginState> Plugins = new(StringComparer.Ordinal);
    private static NativeLogDelegate? _log;
    private static NativeSendMessageDelegate? _sendMessage;
    private static NativeGetUserCountDelegate? _getUserCount;
    private static NativeGetUserInfoDelegate? _getUserInfo;
    private static NativeRegisterCallbackDelegate? _registerCallback;
    private static int _nextHandlerId;

    public static int Initialize(IntPtr args, int sizeBytes)
    {
        try
        {
            if (sizeBytes != Marshal.SizeOf<NativeHostApi>())
            {
                return 1;
            }

            var api = Marshal.PtrToStructure<NativeHostApi>(args);
            _log = Marshal.GetDelegateForFunctionPointer<NativeLogDelegate>(api.Log);
            _sendMessage = Marshal.GetDelegateForFunctionPointer<NativeSendMessageDelegate>(api.SendMessage);
            _getUserCount = Marshal.GetDelegateForFunctionPointer<NativeGetUserCountDelegate>(api.GetUserCount);
            _getUserInfo = Marshal.GetDelegateForFunctionPointer<NativeGetUserInfoDelegate>(api.GetUserInfo);
            _registerCallback = Marshal.GetDelegateForFunctionPointer<NativeRegisterCallbackDelegate>(api.RegisterCallback);
            return 0;
        }
        catch (Exception ex)
        {
            LogException("Initialise bridge", ex);
            return 1;
        }
    }

    public static int LoadPlugin(IntPtr args, int sizeBytes)
    {
        try
        {
            if (sizeBytes != Marshal.SizeOf<LoadPluginRequest>())
            {
                return 1;
            }

            var request = Marshal.PtrToStructure<LoadPluginRequest>(args);
            var pluginId = Marshal.PtrToStringUTF8(request.PluginId);
            var assemblyPath = Marshal.PtrToStringUTF8(request.AssemblyPath);

            if (string.IsNullOrWhiteSpace(pluginId) || string.IsNullOrWhiteSpace(assemblyPath))
            {
                return 1;
            }

            if (Plugins.ContainsKey(pluginId))
            {
                Log($"[C#:{pluginId}] plugin is already loaded.");
                return 1;
            }

            var loadContext = new PluginLoadContext(assemblyPath);
            var assembly = loadContext.LoadFromAssemblyPath(assemblyPath);
            var pluginType = assembly
                .GetTypes()
                .FirstOrDefault(type => typeof(IZoiteChatPlugin).IsAssignableFrom(type) && !type.IsAbstract && !type.IsInterface);

            if (pluginType is null)
            {
                Log($"[C#:{pluginId}] no IZoiteChatPlugin implementation found in {assemblyPath}.");
                loadContext.Unload();
                return 1;
            }

            var plugin = (IZoiteChatPlugin?)Activator.CreateInstance(pluginType);
            if (plugin is null)
            {
                Log($"[C#:{pluginId}] failed to create plugin instance for {pluginType.FullName}.");
                loadContext.Unload();
                return 1;
            }

            var state = new ManagedPluginState(pluginId, loadContext, plugin);
            Plugins[pluginId] = state;

            var context = new ZoiteChatContext(
                text => Log($"[C#:{pluginId}] {text}"),
                (target, text) => SendMessage(state, target, text),
                () => GetUserCount(state),
                () => GetUserInfo(state),
                (eventName, handler) => RegisterCallback(state, eventName, handler));

            plugin.Init(context);
            return 0;
        }
        catch (Exception ex)
        {
            LogException("Load plugin", ex);
            return 1;
        }
    }

    public static int DispatchCallback(IntPtr args, int sizeBytes)
    {
        try
        {
            if (sizeBytes != Marshal.SizeOf<DispatchCallbackRequest>())
            {
                return 1;
            }

            var request = Marshal.PtrToStructure<DispatchCallbackRequest>(args);
            var pluginId = Marshal.PtrToStringUTF8(request.PluginId);
            var handlerName = Marshal.PtrToStringUTF8(request.HandlerName);
            var eventName = Marshal.PtrToStringUTF8(request.EventName);
            var payloadJson = Marshal.PtrToStringUTF8(request.PayloadJson) ?? "{}";

            if (string.IsNullOrWhiteSpace(pluginId)
                || string.IsNullOrWhiteSpace(handlerName)
                || string.IsNullOrWhiteSpace(eventName)
                || !Plugins.TryGetValue(pluginId, out var state)
                || !state.Callbacks.TryGetValue(handlerName, out var handler))
            {
                return 1;
            }

            handler(ZoiteChatEvent.FromJson(eventName, payloadJson));
            return 0;
        }
        catch (Exception ex)
        {
            LogException("Dispatch callback", ex);
            return 1;
        }
    }

    public static int Shutdown(IntPtr args, int sizeBytes)
    {
        _ = args;
        _ = sizeBytes;

        foreach (var state in Plugins.Values)
        {
            try
            {
                state.Callbacks.Clear();
                state.LoadContext.Unload();
            }
            catch (Exception ex)
            {
                LogException($"Shutdown {state.PluginId}", ex);
            }
        }

        Plugins.Clear();
        return 0;
    }

    private static void RegisterCallback(ManagedPluginState state, string eventName, ZoiteChatEventHandler handler)
    {
        if (_registerCallback is null)
        {
            throw new InvalidOperationException("The native callback registry is not available.");
        }

        var handlerName = $"csharp:{state.PluginId}:{Interlocked.Increment(ref _nextHandlerId)}";
        state.Callbacks[handlerName] = handler;

        using var pluginId = new Utf8StringHandle(state.PluginId);
        using var eventNameHandle = new Utf8StringHandle(eventName);
        using var handlerNameHandle = new Utf8StringHandle(handlerName);

        if (_registerCallback(pluginId.Pointer, eventNameHandle.Pointer, handlerNameHandle.Pointer) == 0)
        {
            state.Callbacks.Remove(handlerName);
            throw new InvalidOperationException($"Callback registration failed for event '{eventName}'.");
        }
    }

    private static bool SendMessage(ManagedPluginState state, string target, string text)
    {
        if (_sendMessage is null)
        {
            throw new InvalidOperationException("The native send-message callback is not available.");
        }

        using var pluginIdHandle = new Utf8StringHandle(state.PluginId);
        using var targetHandle = new Utf8StringHandle(target);
        using var textHandle = new Utf8StringHandle(text);
        return _sendMessage(pluginIdHandle.Pointer, targetHandle.Pointer, textHandle.Pointer) != 0;
    }

    private static int GetUserCount(ManagedPluginState state)
    {
        if (_getUserCount is null)
        {
            throw new InvalidOperationException("The native user-count callback is not available.");
        }

        using var pluginIdHandle = new Utf8StringHandle(state.PluginId);
        return checked((int)_getUserCount(pluginIdHandle.Pointer));
    }

    private static ZoiteChatUserInfo GetUserInfo(ManagedPluginState state)
    {
        if (_getUserInfo is null)
        {
            throw new InvalidOperationException("The native user-info callback is not available.");
        }

        using var pluginIdHandle = new Utf8StringHandle(state.PluginId);
        if (_getUserInfo(pluginIdHandle.Pointer, out var userInfo) == 0)
        {
            return new ZoiteChatUserInfo(null, null, null, null);
        }

        return new ZoiteChatUserInfo(
            Marshal.PtrToStringUTF8(userInfo.Nickname),
            Marshal.PtrToStringUTF8(userInfo.Channel),
            Marshal.PtrToStringUTF8(userInfo.ServerName),
            Marshal.PtrToStringUTF8(userInfo.NetworkName));
    }

    private static void Log(string message)
    {
        if (_log is null || string.IsNullOrWhiteSpace(message))
        {
            return;
        }

        using var textHandle = new Utf8StringHandle(message);
        _log(textHandle.Pointer);
    }

    private static void LogException(string operation, Exception ex)
    {
        Log($"{operation} failed: {ex.Message}");
    }

    private sealed class Utf8StringHandle : IDisposable
    {
        public Utf8StringHandle(string value)
        {
            Pointer = Marshal.StringToCoTaskMemUTF8(value);
        }

        public IntPtr Pointer { get; }

        public void Dispose()
        {
            if (Pointer != IntPtr.Zero)
            {
                Marshal.FreeCoTaskMem(Pointer);
            }
        }
    }
}
