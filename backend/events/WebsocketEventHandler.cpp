#include "WebsocketEventHandler.h"

#include "Events.h"
#include "SocketEvents.h"

PostEventState DeviceWebsocketEventHandler::operator()(const Events::DeviceChangeEvent& event)
{
    WebsocketChannel& devicesChannel = m_devicesChannel.Get();
    if (event.GetChangedFields() != Events::DeviceFields::REMOVE && event.GetChanged().GetId().GetValue() != 0)
    {
        const Device& device = event.GetChanged();
        devicesChannel.Broadcast(nlohmann::json{{"device", device.ToJson()}});
        return PostEventState::handled;
    }
    else if (event.GetChangedFields() == Events::DeviceFields::REMOVE)
    {
        devicesChannel.Broadcast(nlohmann::json{{"deleteDevice", event.GetOld().GetId().GetValue()}});
        return PostEventState::handled;
    }
    return PostEventState::notHandled;
}

PostEventState DeviceWebsocketEventHandler::operator()(const Events::DevicePropertyChangeEvent& event)
{
    WebsocketChannel& devicesChannel = m_devicesChannel.Get();
    const Device& device = event.GetChanged();
    if (device.GetId().GetValue() != 0 && !event.GetChangedFields().empty())
    {
        devicesChannel.Broadcast(nlohmann::json{{"propertyChange",
            {{"deviceId", device.GetId().GetValue()}, {"propertyKey", event.GetChangedFields()},
                {"value", device.GetProperty(event.GetChangedFields())}}}});
		return PostEventState::handled;
    }
	return PostEventState::notHandled;
}

PostEventState ActionWebsocketEventHandler::operator()(const Events::ActionChangeEvent& event)
{
    WebsocketChannel& actionsChannel = m_actionsChannel.Get();
    if (event.GetChangedFields() != Events::ActionFields::REMOVE && event.GetChanged().GetId() != 0)
    {
        const Action& action = event.GetChanged();
        actionsChannel.Broadcast(nlohmann::json{{"action", action.ToJson()}});
        return PostEventState::handled;
    }
    else if (event.GetChangedFields() == Events::ActionFields::REMOVE)
    {
        actionsChannel.Broadcast(nlohmann::json{{"deleteAction", event.GetOld().GetId()}});
        return PostEventState::handled;
    }
	return PostEventState::notHandled;
}

PostEventState RuleWebsocketEventHandler::operator()(const Events::RuleChangeEvent& event)
{
    WebsocketChannel& rulesChannel = m_rulesChannel.Get();
    if (event.GetChangedFields() != Events::RuleFields::REMOVE && event.GetChanged().GetId() != 0)
    {
        const Rule& rule = event.GetChanged();
        rulesChannel.Broadcast(nlohmann::json{{"rule", rule.ToJson()}});
        return PostEventState::handled;
    }
    else if (event.GetChangedFields() == Events::RuleFields::REMOVE)
    {
        rulesChannel.Broadcast(nlohmann::json{{"deleteRule", event.GetOld().GetId()}});
        return PostEventState::handled;
    }

    return PostEventState::notHandled;
}
