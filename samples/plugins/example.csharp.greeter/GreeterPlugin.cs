using Fabulor.Plugins;

public sealed class GreeterPlugin : IZoiteChatPlugin
{
    private ZoiteChatContext? _context;
    private bool _reportedFirstMessage;

    public void Init(ZoiteChatContext context)
    {
        _context = context;
        var user = context.GetUserInfo();
        context.Log($"Hello, {user.Nickname ?? "unknown"}. C# sample ready.");
        context.RegisterCallback("message", OnMessage);
    }

    private void OnMessage(ZoiteChatEvent evt)
    {
        if (_reportedFirstMessage || _context is null)
        {
            return;
        }

        _reportedFirstMessage = true;
        var location = string.IsNullOrWhiteSpace(evt.Channel)
            ? "the active session"
            : evt.Channel;
        _context.Log($"C# sample observed its first incoming message event in {location}.");
    }
}
