namespace Fabulor.Plugins;

public sealed class ZoiteChatUserInfo
{
    public ZoiteChatUserInfo(string? nickname, string? channel, string? serverName, string? networkName)
    {
        Nickname = nickname;
        Channel = channel;
        ServerName = serverName;
        NetworkName = networkName;
    }

    public string? Nickname { get; }

    public string? Channel { get; }

    public string? ServerName { get; }

    public string? NetworkName { get; }
}
