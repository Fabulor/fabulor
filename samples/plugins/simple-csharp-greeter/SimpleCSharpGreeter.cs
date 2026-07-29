using Fabulor.Plugins;

public sealed class SimpleCSharpGreeter : IFabulorPlugin
{
    public void Init(FabulorContext context)
    {
        var nickname = context.GetUserInfo().Nickname ?? "unknown";
        context.Log($"Hello, {nickname}. Simple C# add-on ready.");
    }
}
