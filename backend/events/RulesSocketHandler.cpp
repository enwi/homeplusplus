#include "RulesSocketHandler.h"

#include "Events.h"

constexpr const char* RulesSocketHandler::s_addRule;
constexpr const char* RulesSocketHandler::s_getRules;
constexpr const char* RulesSocketHandler::s_getRule;
constexpr const char* RulesSocketHandler::s_removeRule;
constexpr const char* RulesSocketHandler::s_getConditionTypes;

RulesSocketHandler::RulesSocketHandler(const RuleStorage& ruleStorage) : m_ruleStorage(ruleStorage) {}

PostEventState RulesSocketHandler::operator()(const WebsocketChannel::EventVariant& event, WebsocketChannel& channel)
{
    if (absl::holds_alternative<Events::SocketMessageEvent>(event))
    {
        return HandleSocketMessage(absl::get<Events::SocketMessageEvent>(event), channel);
    }
    return PostEventState::notHandled;
}

PostEventState RulesSocketHandler::HandleSocketMessage(
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
        // The command is to add a rule
        if (command == s_addRule)
        {
            nlohmann::json ruleJson = payload.at("ruleJSON");
            Rule rule = Rule::Parse(ruleJson);
            m_ruleStorage.AddRule(rule, event.GetUser().value());
            return PostEventState::handled;
        }
        else if (command == s_getRules)
        {
            std::vector<Rule> rules = m_ruleStorage.GetAllRules(Filter(), event.GetUser().value());
            for (const Rule& rule : rules)
            {
                channel.Send(event.GetConnection(), nlohmann::json {{"rule", rule.ToJson()}});
            }
            if (rules.empty())
            {
                channel.Send(event.GetConnection(), nlohmann::json {{"rules", nlohmann::json::array()}});
            }
            return PostEventState::handled;
        }
        else if (command == s_getRule)
        {
            absl::optional<Rule> rule = m_ruleStorage.GetRule(payload.at("ruleId"), event.GetUser().value());
            if (rule)
            {
                channel.Send(event.GetConnection(), nlohmann::json {{"rule", rule->ToJson()}});
            }
            else
            {
                // TODO: send rule not found
            }
            return PostEventState::handled;
        }
        else if (command == s_removeRule)
        {
            m_ruleStorage.RemoveRule(payload.at("ruleId"), event.GetUser().value());
            return PostEventState::handled;
        }
        else if (command == s_getConditionTypes)
        {
            const auto& registered = Res::ConditionRegistry().GetRegistered();
            nlohmann::json result = nlohmann::json::array();
            for (std::size_t i = 0; i < registered.size(); ++i)
            {
                if (registered[i] != nullptr)
                {
                    result.push_back(nlohmann::json {{"id", i}, {"name", registered[i].name}});
                }
            }
            channel.Send(event.GetConnection(), nlohmann::json {{"conditionTypes", result}});
            return PostEventState::handled;
        }
    }
    catch (const std::exception& e)
    {
        Res::Logger().Error("RulesSocketHandler", std::string("Exception while processing message: ") + e.what());
        return PostEventState::error;
    }
    return PostEventState::notHandled;
}
