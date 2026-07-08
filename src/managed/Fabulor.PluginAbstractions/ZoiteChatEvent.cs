using System.Text.Json.Nodes;

namespace Fabulor.Plugins;

public sealed class ZoiteChatEvent
{
    public ZoiteChatEvent(string eventName, string payloadJson, JsonObject payload)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(eventName);
        ArgumentNullException.ThrowIfNull(payloadJson);
        ArgumentNullException.ThrowIfNull(payload);

        EventName = eventName;
        PayloadJson = payloadJson;
        Payload = payload;
    }

    public string EventName { get; }

    public string PayloadJson { get; }

    public JsonObject Payload { get; }

    public string? Source => Payload["source"]?.GetValue<string>();

    public string? Channel => Payload["channel"]?.GetValue<string>();

    public string? Network => Payload["network"]?.GetValue<string>();

    public string? Nick => Payload["nick"]?.GetValue<string>();

    public string? Server => Payload["server"]?.GetValue<string>();

    public long Time => Payload["time"]?.GetValue<long>() ?? 0;

    public string? Word1 => Payload["word1"]?.GetValue<string>();

    public string? Word2 => Payload["word2"]?.GetValue<string>();

    public string? Word3 => Payload["word3"]?.GetValue<string>();

    public string? Word4 => Payload["word4"]?.GetValue<string>();

    public string? WordEol1 => Payload["word_eol1"]?.GetValue<string>();

    public string? WordEol2 => Payload["word_eol2"]?.GetValue<string>();

    public static ZoiteChatEvent FromJson(string eventName, string payloadJson)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(eventName);
        ArgumentNullException.ThrowIfNull(payloadJson);

        var payloadNode = JsonNode.Parse(payloadJson) as JsonObject ?? new JsonObject();
        return new ZoiteChatEvent(eventName, payloadJson, payloadNode);
    }
}
