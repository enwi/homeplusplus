#include "ActionsSocketHandler.h"

#include "Events.h"

constexpr const char* ActionsSocketHandler::s_addAction;
constexpr const char* ActionsSocketHandler::s_getActions;
constexpr const char* ActionsSocketHandler::s_getAction;
constexpr const char* ActionsSocketHandler::s_deleteAction;
constexpr const char* ActionsSocketHandler::s_execAction;
constexpr const char* ActionsSocketHandler::s_getActionTypes;

ActionsSocketHandler::ActionsSocketHandler(SubActionRegistry& actionRegistry, const ActionStorage& actionStorage,
    DeviceRegistry& deviceReg, WebsocketChannelAccessor notificationsChannelAccessor)
    : m_subActionRegistry(&actionRegistry),
      m_actionStorage(actionStorage),
      m_deviceReg(&deviceReg),
      m_channelAccessor(std::move(notificationsChannelAccessor))
{}

PostEventState ActionsSocketHandler::operator()(const WebsocketChannel::EventVariant& event, WebsocketChannel& channel)
{
    if (absl::holds_alternative<Events::SocketMessageEvent>(event))
    {
        return HandleSocketMessage(absl::get<Events::SocketMessageEvent>(event), channel);
    }
    return PostEventState::notHandled;
}

PostEventState ActionsSocketHandler::HandleSocketMessage(
    const Events::SocketMessageEvent& event, WebsocketChannel& channel)
{
    try
    {
        const nlohmann::json& payload = event.GetJsonPayload();
        auto it = payload.find("command");
        if (it == payload.end())
        {
            return PostEventState::notHandled;
        }
        const std::string& command = *it;
        if (command == s_addAction)
        {
            const nlohmann::json& value = payload.at("actionJSON");
            Action action = Action::Parse(value, *m_subActionRegistry);
            m_actionStorage.AddAction(action, event.GetUser().value());

            return PostEventState::handled;
        }
        else if (command == s_getActions)
        {
            std::vector<Action> actionList = m_actionStorage.GetAllActions(Filter(), event.GetUser().value());
            for (const Action& action : actionList)
            {
                channel.Send(event.GetConnection(), nlohmann::json {{"action", action.ToJson()}});
            }
            if (actionList.empty())
            {
                channel.Send(event.GetConnection(), nlohmann::json {{"actions", nlohmann::json::array()}});
            }
            return PostEventState::handled;
        }
        else if (command == s_getAction)
        {
            absl::optional<Action> action = m_actionStorage.GetAction(payload.at("id"), event.GetUser().value());
            if (action)
            {
                channel.Send(event.GetConnection(), nlohmann::json {{"action", action->ToJson()}});
            }
            else
            {
                // TODO: Send action not found
            }
            return PostEventState::handled;
        }
        else if (command == s_deleteAction)
        {
            absl::optional<Action> action = m_actionStorage.GetAction(payload.at("id"), event.GetUser().value());
            if (action)
            {
                m_actionStorage.RemoveAction(action->GetId(), event.GetUser().value());
            }
            return PostEventState::handled;
        }
        else if (command == s_execAction)
        {
            WebsocketChannel& notificationsChannel = m_channelAccessor.Get();
            absl::optional<Action> action = m_actionStorage.GetAction(payload.at("id"), event.GetUser().value());
            if (action)
            {
                action->Execute(m_actionStorage, notificationsChannel, *m_deviceReg, event.GetUser().value());
            }
            return PostEventState::handled;
        }
        else if (command == s_getActionTypes)
        {
            const std::vector<SubActionInfo>& registered = m_subActionRegistry->GetRegistered();
            nlohmann::json result = nlohmann::json::array();
            for (std::size_t i = 0; i < registered.size(); ++i)
            {
                if (registered[i] != nullptr)
                {
                    result.push_back(nlohmann::json {{"id", i}, {"name", registered[i].name}});
                }
            }
            channel.Send(event.GetConnection(), nlohmann::json {{"actionTypes", result}});
            return PostEventState::handled;
        }
    }
    catch (const std::exception& e)
    {
        Res::Logger().Error("ActionsSocketHandler", std::string("Exception while processing message: ") + e.what());
        return PostEventState::error;
    }
    return PostEventState::notHandled;
}
