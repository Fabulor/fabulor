namespace Fabulor.Plugins;

public delegate void FabulorEventHandler(FabulorEvent evt);

public sealed class FabulorContext
{
    private readonly Action<string> _log;
    private readonly Func<string, string, bool> _sendMessage;
    private readonly Func<int> _getUserCount;
    private readonly Func<FabulorUserInfo> _getUserInfo;
    private readonly Action<string, FabulorEventHandler> _registerCallback;

    public FabulorContext(
        Action<string> log,
        Func<string, string, bool> sendMessage,
        Func<int> getUserCount,
        Func<FabulorUserInfo> getUserInfo,
        Action<string, FabulorEventHandler> registerCallback)
    {
        _log = log ?? throw new ArgumentNullException(nameof(log));
        _sendMessage = sendMessage ?? throw new ArgumentNullException(nameof(sendMessage));
        _getUserCount = getUserCount ?? throw new ArgumentNullException(nameof(getUserCount));
        _getUserInfo = getUserInfo ?? throw new ArgumentNullException(nameof(getUserInfo));
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

    public FabulorUserInfo GetUserInfo()
    {
        return _getUserInfo();
    }

    public void RegisterCallback(string eventName, FabulorEventHandler handler)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(eventName);
        ArgumentNullException.ThrowIfNull(handler);
        _registerCallback(eventName, handler);
    }
}
