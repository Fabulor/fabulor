using Fabulor.Plugins;

public sealed class SimpleCSharpGreeter : IZoiteChatPlugin
{
    public void Init(ZoiteChatContext context)
    {
        var nickname = context.GetUserInfo().Nickname ?? "unknown";
        context.Log($"Hello, {nickname}. Simple C# add-on ready.");
    }
}
