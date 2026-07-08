using Fabulor.Plugins;

public sealed class GreeterPlugin : IZoiteChatPlugin
{
    public void Init(ZoiteChatContext ctx)
    {
        var user = ctx.GetUserInfo();
        var target = string.IsNullOrWhiteSpace(user.Channel) ? "#fabulor" : user.Channel;
        ctx.Log($"C# sample plugin initialised for {user.Nickname ?? "unknown"}");
        ctx.SendMessage(target, "Hello from the C# sample plugin");
        ctx.RegisterCallback("message", OnMessage);
    }

    private static void OnMessage(ZoiteChatEvent evt)
    {
    }
}
