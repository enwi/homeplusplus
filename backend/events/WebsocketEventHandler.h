#ifndef _WEBSOCKET_EVENT_HANDLER_H
#define _WEBSOCKET_EVENT_HANDLER_H
#include <json.hpp>

#include "EventSystem.h"
#include "Events.h"

#include "../communication/WebsocketCommunication.h"

// Handles ChangeEvents to update websocket clients
class DeviceWebsocketEventHandler
{
public:
    DeviceWebsocketEventHandler(WebsocketChannelAccessor devicesChannel)
        : m_devicesChannel(devicesChannel)
    {}

	PostEventState operator()(const Events::DeviceChangeEvent& event);
	PostEventState operator()(const Events::DevicePropertyChangeEvent& event);

private:
    WebsocketChannelAccessor m_devicesChannel;    
	
};

class ActionWebsocketEventHandler
{
public:
	ActionWebsocketEventHandler(WebsocketChannelAccessor actionsChannel)
		: m_actionsChannel(actionsChannel)
	{}

	PostEventState operator()(const Events::ActionChangeEvent& event);

private:
	WebsocketChannelAccessor m_actionsChannel;
};

class RuleWebsocketEventHandler
{
public:
	RuleWebsocketEventHandler(WebsocketChannelAccessor rulesChannel)
		: m_rulesChannel(rulesChannel)
	{}

	PostEventState operator()(const Events::RuleChangeEvent& event);

private:
	WebsocketChannelAccessor m_rulesChannel;
};

#endif
