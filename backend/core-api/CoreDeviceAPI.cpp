#include "CoreDeviceAPI.h"

#include "../database/DBEventHandler.h"
#include "../events/ActionsSocketHandler.h"
#include "../events/AuthEventHandler.h"
#include "../events/DevicesSocketHandler.h"
#include "../events/ProfileSocketHandler.h"
#include "../events/RuleEventHandler.h"
#include "../events/RulesSocketHandler.h"
#include "../events/WebsocketEventHandler.h"

void CoreDeviceAPI::Initialize(nlohmann::json&) {}

void CoreDeviceAPI::Start()
{
    if (m_loadReqHandler)
    {
        m_loadReqHandler->StartThread();
    }
}

void CoreDeviceAPI::Shutdown()
{
    if (m_loadReqHandler)
    {
        m_loadReqHandler->StopThread();
    }
}

void CoreDeviceAPI::RegisterEventHandlers(EventSystem& evSys)
{
    WebsocketChannel statsChannel(*m_sockComm, "stats", WebsocketChannel::RequireAuth::auth);
    WebsocketChannel devicesChannel(*m_sockComm, "devices", WebsocketChannel::RequireAuth::auth);
    WebsocketChannel rulesChannel(*m_sockComm, "rules", WebsocketChannel::RequireAuth::auth);
    WebsocketChannel actionsChannel(*m_sockComm, "actions", WebsocketChannel::RequireAuth::auth);
    WebsocketChannel notificationsChannel(*m_sockComm, "notifications", WebsocketChannel::RequireAuth::auth);
    WebsocketChannel profileChannel(*m_sockComm, "profile", WebsocketChannel::RequireAuth::auth);

    WebsocketChannelAccessor notificationsAccessor(*m_sockComm, notificationsChannel.GetName());

    m_sockComm->AddEventHandler(AuthEventHandler(*m_dbHandler, *m_sockComm, *m_authenticator));

    // evSys.AddHandler(std::make_shared<WebsocketEventHandler>(devicesChannel, rulesChannel, actionsChannel));
    // evSys.AddHandler(std::make_shared<DBEventHandler>(*m_dbHandler, *m_actionSer, *m_ruleSer));
    m_loadReqHandler
        = std::make_unique<LoadRequestHandler>(WebsocketChannelAccessor(*m_sockComm, statsChannel.GetName()));

    statsChannel.AddEventHandler([this](const WebsocketChannel::EventVariant& e, WebsocketChannel& channel) {
        return m_loadReqHandler->HandleEvent(e, channel);
    });
    devicesChannel.AddEventHandler(DevicesSocketHandler(*m_dbHandler, m_deviceReg->GetStorage(), *m_deviceTypeReg));
    actionsChannel.AddEventHandler(ActionsSocketHandler(
        Res::ActionRegistry(), ActionStorage(*m_actionSer, m_actionChanges), *m_deviceReg, notificationsAccessor));
    rulesChannel.AddEventHandler(RulesSocketHandler(RuleStorage(*m_ruleSer, m_ruleChanges)));
    profileChannel.AddEventHandler(ProfileSocketHandler(*m_dbHandler, *m_authenticator));

    m_actionChanges.AddHandler(
        ActionWebsocketEventHandler(WebsocketChannelAccessor(*m_sockComm, actionsChannel.GetName())));
    m_ruleChanges.AddHandler(RuleWebsocketEventHandler(WebsocketChannelAccessor(*m_sockComm, rulesChannel.GetName())));
    m_deviceChanges->AddHandler(
        DeviceWebsocketEventHandler(WebsocketChannelAccessor(*m_sockComm, devicesChannel.GetName())));
    m_propertyChanges->AddHandler(
        DeviceWebsocketEventHandler(WebsocketChannelAccessor(*m_sockComm, devicesChannel.GetName())));

    m_sockComm->AddChannel(std::move(statsChannel));
    m_sockComm->AddChannel(std::move(devicesChannel));
    m_sockComm->AddChannel(std::move(rulesChannel));
    m_sockComm->AddChannel(std::move(actionsChannel));
    m_sockComm->AddChannel(std::move(notificationsChannel));
    m_sockComm->AddChannel(std::move(profileChannel));

    evSys.AddHandler(std::make_shared<RuleEventHandler>(
        ActionStorage(*m_actionSer, m_actionChanges), notificationsAccessor, *m_deviceReg, *m_ruleSer));
}

void CoreDeviceAPI::RegisterRuleConditions(RuleConditions::Registry& registry)
{
    registry.RegisterDefaultConditions();
}

void CoreDeviceAPI::RegisterSubActions(SubActionRegistry& registry)
{
    registry.RegisterDefaultSubActions();
}