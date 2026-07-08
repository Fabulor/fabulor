namespace Fabulor.Plugins;

public delegate void ZoiteChatEventHandler(ZoiteChatEvent evt);

public sealed class ZoiteChatContext
{
    private readonly Action<string> _log;
    private readonly Func<string, string, bool> _sendMessage;
    private readonly Func<int> _getUserCount;
    private readonly Action<string, ZoiteChatEventHandler> _registerCallback;

    public ZoiteChatContext(
        Action<string> log,
        Func<string, string, bool> sendMessage,
        Func<int> getUserCount,
        Action<string, ZoiteChatEventHandler> registerCallback)
    {
        _log = log ?? throw new ArgumentNullException(nameof(log));
        _sendMessage = sendMessage ?? throw new ArgumentNullException(nameof(sendMessage));
        _getUserCount = getUserCount ?? throw new ArgumentNullException(nameof(getUserCount));
        _registerCallback = registerCallback ?? throw new ArgumentNullException(nameof(registerCallback));
    }

    public void Log(string text)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(text);
        _log(text);
    }

    public bool SendMessage(string target, string text)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(target);
        ArgumentException.ThrowIfNullOrWhiteSpace(text);
        return _sendMessage(target, text);
    }

    public int GetUserCount()
    {
        return _getUserCount();
    }

    public void RegisterCallback(string eventName, ZoiteChatEventHandler handler)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(eventName);
        ArgumentNullException.ThrowIfNull(handler);
        _registerCallback(eventName, handler);
    }
}
