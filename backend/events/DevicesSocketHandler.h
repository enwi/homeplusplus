#ifndef DEVICES_SOCKET_HANDLER_H
#define DEVICES_SOCKET_HANDLER_H

#include "../api/DeviceRegistry.h"
#include "../communication/WebsocketChannel.h"
#include "../database/DBHandler.h"

class DevicesSocketHandler
{
public:
    DevicesSocketHandler(DBHandler& dbHandler, DeviceStorage& deviceStorage, DeviceTypeRegistry& typeRegistry);

    PostEventState operator()(const WebsocketChannel::EventVariant& event, WebsocketChannel& channel);

    PostEventState HandleSocketMessage(const Events::SocketMessageEvent& event, WebsocketChannel& channel);

public:
    static constexpr const char* s_getDevices = "GET_DEVICES";
    static constexpr const char* s_getDevice = "GET_DEVICE";
    static constexpr const char* s_getSensorData = "GET_SENSOR_DATA";
    static constexpr const char* s_deleteDevice = "DELETE_DEVICE";
    static constexpr const char* s_setName = "SET_DEVICE_NAME";
    static constexpr const char* s_setGroups = "SET_DEVICE_GROUPS";
    static constexpr const char* s_setProperty = "SET_DEVICE_PROPERTY";
    static constexpr const char* s_getMeta = "GET_TYPE_META";
    static constexpr const char* s_getAllMeta = "GET_ALL_TYPES";
    static constexpr const char* s_getPropertyLog = "GET_PROPERTY_LOG";

private:
    PostEventState SendSensorData(
        Events::SocketMessageEvent::connection_hdl hdl, const nlohmann::json& request, WebsocketChannel& channel);

    nlohmann::json BuildMetaJson(const std::string& type) const;

    nlohmann::json BuildPropertyLogJson(const DeviceId deviceId, const nlohmann::json& properties) const;

private:
    DBHandler* m_dbHandler;
    DeviceStorage* m_deviceStorage;
    DeviceTypeRegistry* m_typeRegistry;
};

#endif
