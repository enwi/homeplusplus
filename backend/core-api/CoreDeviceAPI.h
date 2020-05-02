#pragma once

#include "../api/ActionStorage.h"
#include "../api/DeviceAPI.h"
#include "../api/IActionSerialize.h"
#include "../api/IRuleSerialize.h"
#include "../api/RuleStorage.h"
#include "../communication/Authenticator.h"
#include "../communication/WebsocketCommunication.h"
#include "../database/DBHandler.h"
#include "../events/LoadRequestHandler.h"

class CoreDeviceAPI : public IDeviceAPI
{
public:
    CoreDeviceAPI(DBHandler& dbHandler, DeviceRegistry& deviceReg, DeviceTypeRegistry& deviceTypeReg,
        WebsocketCommunication& sockComm, IActionSerialize& actionSerialize, IRuleSerialize& ruleSerialize,
        Authenticator& authenticator, EventEmitter<Events::DeviceChangeEvent>& deviceChanges,
        EventEmitter<Events::DevicePropertyChangeEvent>& propertyChanges)
        : m_dbHandler(&dbHandler),
          m_deviceReg(&deviceReg),
          m_deviceTypeReg(&deviceTypeReg),
          m_sockComm(&sockComm),
          m_actionSer(&actionSerialize),
          m_ruleSer(&ruleSerialize),
          m_authenticator(&authenticator),
          m_deviceChanges(&deviceChanges),
          m_propertyChanges(&propertyChanges),
          m_actionStorage(actionSerialize, m_actionChanges),
          m_ruleStorage(ruleSerialize, m_ruleChanges)
    {}

    void Initialize(nlohmann::json& config) override;

    void Start() override;

    void Shutdown() override;

    void RegisterEventHandlers(EventSystem& evSys) override;

    void RegisterRuleConditions(RuleConditions::Registry& registry) override;

    void RegisterSubActions(SubActionRegistry& registry) override;

    const char* GetAPIId() const noexcept override { return "CORE"; }

private:
    DBHandler* m_dbHandler;
    DeviceRegistry* m_deviceReg;
    DeviceTypeRegistry* m_deviceTypeReg;
    WebsocketCommunication* m_sockComm;
    IActionSerialize* m_actionSer;
    IRuleSerialize* m_ruleSer;
    Authenticator* m_authenticator;
    std::unique_ptr<LoadRequestHandler> m_loadReqHandler;
    EventEmitter<Events::RuleChangeEvent> m_ruleChanges;
    EventEmitter<Events::ActionChangeEvent> m_actionChanges;
    EventEmitter<Events::DeviceChangeEvent>* m_deviceChanges;
    EventEmitter<Events::DevicePropertyChangeEvent>* m_propertyChanges;
    ActionStorage m_actionStorage;
    RuleStorage m_ruleStorage;
};