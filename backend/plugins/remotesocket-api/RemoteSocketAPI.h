#pragma once

#include "RemoteSocketTransmitter.h"

#include "../api/DeviceAPI.h"
#include "../communication/Authenticator.h"
#include "../communication/WebsocketCommunication.h"
#include "../database/DBHandler.h"

class RemoteSocketAPI : public IDeviceAPI
{
public:
    RemoteSocketAPI(
        UserId apiUser, DBHandler& dbHandler, WebsocketCommunication& sockComm, Authenticator& authenticator)
        : m_apiUser(apiUser), m_dbHandler(&dbHandler), m_sockComm(&sockComm), m_authenticator(&authenticator)
    {}

    void Initialize(nlohmann::json& config) override;
    void RegisterEventHandlers(EventSystem& evSys) override;
    void Shutdown() override;

    void RegisterDeviceTypes(DeviceTypeRegistry& registry) override;
    void SynchronizeDevices(DeviceStorage& storage) override;

    const char* GetAPIId() const noexcept override { return "REMOTESOCKET_API0.0"; }

private:
    UserId m_apiUser;
    DBHandler* m_dbHandler;
    WebsocketCommunication* m_sockComm;
    RemoteSocketTransmitter m_transmitter;
    Authenticator* m_authenticator;
    Metadata m_remoteSocketMetadata;
};