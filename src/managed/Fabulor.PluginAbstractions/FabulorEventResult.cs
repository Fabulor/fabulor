namespace Fabulor.Plugins;

public enum FabulorEventResult
{
    Continue = 0,
    Consume = 1,
}

public delegate FabulorEventResult FabulorConsumingEventHandler(FabulorEvent evt);
