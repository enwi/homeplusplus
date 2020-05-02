#ifndef ACTIONS_SOCKET_HANDLER
#define ACTIONS_SOCKET_HANDLER

#include "../api/Action.h"
#include "../api/ActionStorage.h"
#include "../api/IActionSerialize.h"
#include "../communication/WebsocketCommunication.h"

class ActionsSocketHandler
{
public:
    ActionsSocketHandler(SubActionRegistry& subActionRegistry, const ActionStorage& actionStorage,
        DeviceRegistry& deviceReg, WebsocketChannelAccessor notificationsChannelAccessor);

    PostEventState operator()(const WebsocketChannel::EventVariant& event, WebsocketChannel& channel);

    PostEventState HandleSocketMessage(const Events::SocketMessageEvent& event, WebsocketChannel& channel);

public:
    static constexpr const char* s_addAction = "ADD_ACTION";
    static constexpr const char* s_getActions = "GET_ACTIONS";
    static constexpr const char* s_getAction = "GET_ACTION";
    static constexpr const char* s_deleteAction = "DELETE_ACTION";
    static constexpr const char* s_execAction = "EXEC_ACTION";
    static constexpr const char* s_getActionTypes = "GET_ACTION_TYPES";

private:
    SubActionRegistry* m_subActionRegistry;
    ActionStorage m_actionStorage;
    DeviceRegistry* m_deviceReg;
    WebsocketChannelAccessor m_channelAccessor;
};

#endif
