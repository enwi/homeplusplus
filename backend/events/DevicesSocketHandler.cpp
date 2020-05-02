#include "DevicesSocketHandler.h"

#include <hinnant-date/include/date/date.h>
#include <hinnant-date/include/date/tz.h>

#include "../api/Resources.h"
#include "../database/DevicesTable.h"
#include "../utility/Logger.h"

constexpr const char* DevicesSocketHandler::s_getDevices;
constexpr const char* DevicesSocketHandler::s_getDevice;
constexpr const char* DevicesSocketHandler::s_getSensorData;

DevicesSocketHandler::DevicesSocketHandler(
    DBHandler& dbHandler, DeviceStorage& deviceStorage, DeviceTypeRegistry& typeRegistry)
    : m_dbHandler(&dbHandler), m_deviceStorage(&deviceStorage), m_typeRegistry(&typeRegistry)
{}

PostEventState DevicesSocketHandler::operator()(const WebsocketChannel::EventVariant& event, WebsocketChannel& channel)
{
    if (absl::holds_alternative<Events::SocketMessageEvent>(event))
    {
        return HandleSocketMessage(absl::get<Events::SocketMessageEvent>(event), channel);
    }
    return PostEventState::notHandled;
}

// // time for example: Sat, 25 Apr 2020 13:12:56 GMT
// std::chrono::system_clock::time_point StringToTime(const std::string& str)
// {
//     std::tm tm = {};
//     std::stringstream ss(str);
//     ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");
//     return std::chrono::system_clock::from_time_t(std::mktime(&tm));
// }

absl::optional<std::chrono::system_clock::time_point> getEnd(const nlohmann::json& json)
{
    if (json.count("end"))
    {
        // return StringToTime(json.at("end"));
        const time_t time = json.at("end");
        return std::chrono::system_clock::from_time_t(time);
    }
    else
    {
        return absl::nullopt;
    }
}

PostEventState DevicesSocketHandler::HandleSocketMessage(
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
        UserId user = event.GetUser().value();
        if (command == s_getDevices)
        {
            std::vector<Device> devices = m_deviceStorage->GetAllDevices(user);
            for (const Device& device : devices)
            {
                channel.Send(event.GetConnection(), nlohmann::json {{"device", device.ToJson()}});
            }
            if (devices.empty())
            {
                channel.Send(event.GetConnection(), nlohmann::json {{"devices", nlohmann::json::array()}});
            }
            return PostEventState::handled;
        }
        else if (command == s_getDevice)
        {
            absl::optional<const Device> device
                = m_deviceStorage->GetDevice(DeviceId(payload.at("id").get<int64_t>()), user);
            if (device)
            {
                channel.Send(event.GetConnection(), nlohmann::json {{"device", device->ToJson()}});
            }
            else
            {
                channel.Send(event.GetConnection(), nlohmann::json {{"device", nullptr}});
            }
            return PostEventState::handled;
        }
        else if (command == s_deleteDevice)
        {
            DeviceId id {payload.at("id").get<int64_t>()};
            absl::optional<const Device> device = m_deviceStorage->GetDevice(id, user);
            if (device && m_typeRegistry->HasDeviceType(device->GetType()))
            {
                const DeviceType& type = m_typeRegistry->GetDeviceType(device->GetType());
                if (type.CanRemoveDevice())
                {
                    m_deviceStorage->RemoveDevice(id, user);
                }
                else
                {
                    Res::Logger().Warning("Cannot remove device " + std::to_string(id.GetValue()));
                }
            }
            return PostEventState::handled;
        }
        else if (command == s_setName)
        {
            DeviceId id {payload.at("id").get<int64_t>()};
            std::string name = payload.at("name");
            absl::optional<Device> device = m_deviceStorage->GetDevice(id, user);
            if (!device)
            {
                Res::Logger().Warning("Could not find device " + std::to_string(id.GetValue()));
                return PostEventState::error;
            }
            device->SetName(name);
            m_deviceStorage->ChangeDevice(*device, user);
        }
        else if (command == s_setGroups)
        {
            DeviceId id {payload.at("id").get<int64_t>()};
            nlohmann::json groups = payload.at("groups");
            std::vector<std::string> groupVector;
            groupVector.reserve(groups.size());
            for (const auto& g : groups)
            {
                groupVector.push_back(g);
            }
            absl::optional<Device> device = m_deviceStorage->GetDevice(id, user);
            if (!device)
            {
                Res::Logger().Warning("Could not find device " + std::to_string(id.GetValue()));
                return PostEventState::error;
            }
            device->SetGroups(groupVector);
            m_deviceStorage->ChangeDevice(*device, user);
        }
        else if (command == s_setProperty)
        {
            DeviceId devId {payload.at("id").get<int64_t>()};
            std::string key = payload.at("property");
            nlohmann::json val = payload.at("value");
            absl::optional<Device> device = m_deviceStorage->GetDevice(devId, user);
            if (!device)
            {
                Res::Logger().Warning("Could not find device " + std::to_string(devId.GetValue()));
                return PostEventState::error;
            }
            if (!device->SetProperty(key, val, *m_deviceStorage, user))
            {
                Res::Logger().Warning("Could not set device property");
                return PostEventState::error;
            }

            return PostEventState::handled;
        }
        else if (command == s_getMeta)
        {
            std::string type = payload.at("type");
            channel.Send(event.GetConnection(), BuildMetaJson(type));
        }
        else if (command == s_getAllMeta)
        {
            nlohmann::json arr;
            for (const std::string& type : m_typeRegistry->GetRegisteredTypes())
            {
                arr.emplace_back(BuildMetaJson(type));
            }
            channel.Send(event.GetConnection(), nlohmann::json {{"allTypes", arr}});
        }
        else if (command == s_getPropertyLog)
        {
            // request = {command: "GET_PROPERTY_LOG", properties: {<deviceid> : [properties]}}
            auto type = payload.at("properties");
            const time_t time = payload.at("start");
            auto start = std::chrono::system_clock::from_time_t(time);
            absl::optional<std::chrono::system_clock::time_point> end = getEnd(payload);
            std::time_t compression;
            if (payload.count("compression"))
            {
                compression = payload.at("compression");
            }
            else
            {
                compression = 0;
            }
            nlohmann::json data;
            for (auto it = type.begin(); it != type.end(); ++it)
            {
                DeviceId devId {std::stoi(it.key())};
                nlohmann::json& deviceJson = data[it.key()];

                absl::optional<Device> device = m_deviceStorage->GetDevice(devId, user);
                if (device)
                {
                    for (auto propertyIt : it.value())
                    {
                        deviceJson[propertyIt.get<std::string>()]
                            = device->GetPropertyHistory(propertyIt, start, end, compression, *m_deviceStorage, user);
                    }
                }
            }
            channel.Send(event.GetConnection(), nlohmann::json {{"log", data}});
        }
        /*else if (command == s_getSensorData)
        {
            return SendSensorData(event.GetConnection(), payload);
        }*/
    }
    catch (const std::exception& e)
    {
        Res::Logger().Error("DevicesSocketHandler", std::string("Exception while processing message: ") + e.what());
        return PostEventState::error;
    }
    return PostEventState::notHandled;
}

PostEventState DevicesSocketHandler::SendSensorData(
    Events::SocketMessageEvent::connection_hdl hdl, const nlohmann::json& request, WebsocketChannel& channel)
{
    const auto swapByteOrder = [](int64_t val) {
        return ((((val)&0xff00000000000000ull) >> 56) | (((val)&0x00ff000000000000ull) >> 40)
            | (((val)&0x0000ff0000000000ull) >> 24) | (((val)&0x000000ff00000000ull) >> 8)
            | (((val)&0x00000000ff000000ull) << 8) | (((val)&0x0000000000ff0000ull) << 24)
            | (((val)&0x000000000000ff00ull) << 40) | (((val)&0x00000000000000ffull) << 56));
    };
    // TODO: Make this free of possible padding issues
    /*std::vector<std::pair<long long, long long>> data;
    const int mode = request.at("mode");
    const long long startTime = request.at("startTime");
    const long long endTime = request.at("endTime");
    const int node = request.at("nodeId");
    const int sensor = request.at("sensorId");
    auto& db = m_dbHandler->GetDatabase();
    {
        auto result
            = db(select(sensors.sensorUid).from(sensors).where(sensors.nodeId == node && sensors.sensorId == sensor));

        if (result.NextRow())
        {
            const long long sensorUID = result.GetColumnInt64(0);
            if (mode == 0)
            {
                // Uncompressed data
                result = m_dbHandler
                             ->GetROStatement("SELECT changetime, val FROM sensor_log WHERE sensor_uid = ?1 AND "
                                              "changetime BETWEEN ?2 AND ?3;")
                             .Execute({sensorUID, startTime, endTime});
            }
            else if (mode == 1 || mode == 2)
            {
                // Compressed data
                result = m_dbHandler
                             ->GetROStatement(
                                 "SELECT changetime, val FROM sensor_log JOIN sensor_log_compressed ON "
                                 "sensor_log.rowid = sensor_log_id WHERE sensor_uid = ?1 AND changetime "
                                 "BETWEEN ?2 AND ?3 "
                                 "UNION SELECT changetime,val FROM sensor_log WHERE sensor_uid = ?1 AND "
                                 "changetime BETWEEN "
                                 "min(?3, max(?2, (SELECT changetime FROM sensor_log JOIN "
                                 "sensor_log_compressed ON "
                                 "sensor_log.rowid = sensor_log_compressed.sensor_log_id WHERE sensor_uid = ?1 "
                                 "ORDER BY changetime DESC LIMIT 1))) AND ?3;")
                             .Execute({sensorUID, startTime, endTime});
            }
            else
            {
                // Unknown mode
                return PostEventState::error;
            }
            while (result.NextRow())
            {
                data.emplace_back(swapByteOrder(result.GetColumnInt64(0)), swapByteOrder(result.GetColumnInt64(1)));
            }
        }
    }
    savepoint.Release();
    Res::Logger().Debug("Size: " + std::to_string(data.size()));

    channel.SendBytes(hdl, data.data(), data.size() * sizeof(std::pair<long long, long long>));*/
    return PostEventState::handled;
}

nlohmann::json DevicesSocketHandler::BuildMetaJson(const std::string& type) const
{
    if (!m_typeRegistry->HasDeviceType(type))
    {
        Res::Logger().Warning("Unknown device type: " + type);
        return nlohmann::json {{"typeName", type}, {"meta", nullptr}};
    }
    else
    {
        const Metadata& meta = m_typeRegistry->GetDeviceType(type).GetDeviceMetadata();
        nlohmann::json json = nlohmann::json::object();
        std::vector<std::string> paths = meta.GetEntryPaths();
        for (const std::string& path : paths)
        {
            const MetadataEntry& entry = meta.GetEntry(path);
            json.emplace(path, nlohmann::json {{"dataType", entry.GetType()}, {"access", entry.GetAccess()}});
        }
        return nlohmann::json {{"typeName", type}, {"meta", std::move(json)}};
    }
}

#include "../utility/AnyJson.h"

namespace
{
    constexpr PropertiesLogTable propertiesLogTable;
} // namespace

nlohmann::json DevicesSocketHandler::BuildPropertyLogJson(
    const DeviceId deviceId, const nlohmann::json& properties) const
{
    auto& db = m_dbHandler->GetDatabase();

    nlohmann::json dataJson;
    const int64_t devId = deviceId.GetValue();

    for (auto propertyIt : properties)
    {
        std::string property = propertyIt;
        nlohmann::json& propertyJson = dataJson[property];

        auto result = db(
            select(propertiesLogTable.propertyValue, propertiesLogTable.propertyDate)
                .from(propertiesLogTable)
                .where(propertiesLogTable.deviceId == devId && propertiesLogTable.propertyKey == property
                    && propertiesLogTable.propertyDate >= std::chrono::system_clock::now() - std::chrono::hours(24)));
        for (const auto& row : result)
        {
            nlohmann::json value;
            if (row.propertyValue.is_null())
            {
                value = nullptr;
            }
            else
            {
                google::protobuf::Any any;
                if (!any.ParseFromArray(row.propertyValue.blob, row.propertyValue.len))
                {
                    throw std::runtime_error("DevicesSocketHandler::BuildPropertyLogJson: Invalid blob data");
                }
                value = UnpackAny(any);
            }
            std::time_t time = std::chrono::system_clock::to_time_t(row.propertyDate.value());
            propertyJson[std::ctime(&time)] = value;
            // 2007-08-31T16:47+00:00
            // %Y-%m-%d = %F
            // %FT%T%Ez
            // propertyJson[date::format("%FT%T%Ez",
            //     date::zoned_time<std::chrono::system_clock::duration> {"Etc/UTC", row.propertyDate.value()})]
            //     = value;
        }
    }

    return dataJson;
}
