#ifndef RULES_SOCKET_HANDLER
#define RULES_SOCKET_HANDLER

#include "../api/Action.h"
#include "../api/IRuleSerialize.h"
#include "../api/RuleStorage.h"
#include "../communication/WebsocketChannel.h"

class RulesSocketHandler
{
public:
    explicit RulesSocketHandler(const RuleStorage& ruleStorage);

    PostEventState operator()(const WebsocketChannel::EventVariant& event, WebsocketChannel& channel);

    PostEventState HandleSocketMessage(const Events::SocketMessageEvent& event, WebsocketChannel& channel);

public:
    static constexpr const char* s_addRule = "ADD_RULE";
    static constexpr const char* s_getRules = "GET_RULES";
    static constexpr const char* s_getRule = "GET_RULE";
    static constexpr const char* s_removeRule = "DELETE_RULE";
    static constexpr const char* s_getConditionTypes = "GET_CONDITION_TYPES";

private:
    RuleStorage m_ruleStorage;
};

#endif
