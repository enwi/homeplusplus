#include "DBDeviceSerialize.h"

#include <chrono>

#include <google/protobuf/wrappers.pb.h>
#include <hinnant-date/include/date/tz.h>
#include <sqlpp11/insert.h>
#include <sqlpp11/remove.h>
#include <sqlpp11/select.h>
#include <sqlpp11/transaction.h>
#include <sqlpp11/update.h>

#include "DevicesTable.h"

#include "../utility/AnyJson.h"
#include "../utility/RDPAlgorithm.h"

namespace
{
    constexpr DevicesTable devices;
    constexpr DeviceGroupsTable deviceGroups;
    constexpr PropertiesTable propertiesTable;
    constexpr PropertiesLogTable propertiesLogTable;

    template <typename T, typename Db>
    std::remove_reference_t<T> CommitAndReturn(T&& value, sqlpp::transaction_t<Db>& transaction)
    {
        transaction.commit();
        return std::forward<T>(value);
    }
} // namespace

absl::optional<Device::Data> DBDeviceSerialize::GetDeviceData(DeviceId deviceId, UserId user) const
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(GetDeviceData(deviceId, {user, transaction}), transaction);
}

absl::optional<Device::Data> DBDeviceSerialize::GetDeviceData(
    DeviceId deviceId, const UserHeldTransaction& transaction) const
{
    auto& db = m_dbHandler.GetDatabase();
    auto result = db(select(devices.deviceName, devices.deviceIcon, devices.deviceType, devices.deviceApi)
                         .from(devices)
                         .where(devices.deviceId == deviceId.GetValue()));
    if (result.empty() || !m_deviceTypes.HasDeviceType(result.front().deviceType.value()))
    {
        return absl::nullopt;
    }
    const auto& row = result.front();
    return Device::Data {row.deviceName, row.deviceIcon, GetDeviceGroups(deviceId, transaction), row.deviceType.value(),
        GetDeviceProperties(deviceId, m_deviceTypes.GetDeviceType(row.deviceType.value()), transaction), row.deviceApi};
}

DeviceId DBDeviceSerialize::AddDevice(const Device::Data& deviceData, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(AddDevice(deviceData, {user, transaction}), transaction);
}
DeviceId DBDeviceSerialize::AddDevice(const Device::Data& deviceData, const UserHeldTransaction& transaction)
{
    auto& db = m_dbHandler.GetDatabase();
    uint64_t id
        = db(insert_into(devices).set(devices.deviceName = deviceData.m_name, devices.deviceIcon = deviceData.m_icon,
            devices.deviceApi = deviceData.m_api, devices.deviceType = deviceData.m_type));
    DeviceId deviceId(id);
    InsertDeviceGroups(deviceId, deviceData.m_groups, transaction);
    AddProperties(deviceId, deviceData.m_properties, transaction);
    return deviceId;
}

void DBDeviceSerialize::UpdateDevice(DeviceId id, const Device::Data& data, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    UpdateDevice(id, data, {user, transaction});
    transaction.commit();
}

void DBDeviceSerialize::UpdateDevice(DeviceId id, const Device::Data& data, const UserHeldTransaction& transaction)
{
    auto& db = m_dbHandler.GetDatabase();
    auto result = db(select(devices.deviceId).from(devices).where(devices.deviceId == id.GetValue()));
    if (!result.empty())
    {
        db(remove_from(deviceGroups).where(deviceGroups.deviceId == id.GetValue()));
        db(remove_from(propertiesTable).where(propertiesTable.deviceId == id.GetValue()));
        db(update(devices)
                .set(devices.deviceName = data.m_name, devices.deviceIcon = data.m_icon, devices.deviceApi = data.m_api,
                    devices.deviceType = data.m_type)
                .where(devices.deviceId == id.GetValue()));
        AddProperties(id, data.m_properties, transaction);
        InsertDeviceGroups(id, data.m_groups, transaction);
    }
    else
    {
        throw std::runtime_error("DBDeviceSerialize::UpdateDevice on non-existent device");
    }
}

std::vector<DeviceId> DBDeviceSerialize::GetAPIDeviceIds(absl::string_view apiId, UserId user) const
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(GetAPIDeviceIds(apiId, {user, transaction}), transaction);
}

std::vector<DeviceId> DBDeviceSerialize::GetAPIDeviceIds(absl::string_view apiId, const UserHeldTransaction&) const
{
    auto& db = m_dbHandler.GetDatabase();
    std::string apiIdStr(apiId);
    auto result = db(select(devices.deviceId).from(devices).where(devices.deviceApi == apiIdStr));
    std::vector<DeviceId> ids;
    for (const auto& row : result)
    {
        ids.push_back(DeviceId(row.deviceId));
    }
    return ids;
}

std::vector<DeviceId> DBDeviceSerialize::GetAllDeviceIds(const Filter& filter, UserId user) const
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(GetAllDeviceIds(filter, {user, transaction}), transaction);
}

std::vector<DeviceId> DBDeviceSerialize::GetAllDeviceIds(const Filter& filter, const UserHeldTransaction&) const
{
    auto& db = m_dbHandler.GetDatabase();
    auto result = db(select(devices.deviceId).from(devices).unconditionally());
    std::vector<DeviceId> ids;
    for (const auto& row : result)
    {
        ids.push_back(DeviceId(row.deviceId));
    }
    return ids;
}

void DBDeviceSerialize::RemoveDevice(DeviceId deviceId, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    RemoveDevice(deviceId, {user, transaction});
}

void DBDeviceSerialize::RemoveDevice(DeviceId deviceId, const UserHeldTransaction&)
{
    auto& db = m_dbHandler.GetDatabase();
    // Constraints take care of rest
    db(remove_from(devices).where(devices.deviceId == deviceId.GetValue()));
}

void DBDeviceSerialize::SetDeviceProperty(
    DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    SetDeviceProperty(deviceId, propertyKey, properties, {user, transaction});
    transaction.commit();
}

void DBDeviceSerialize::SetDeviceProperty(
    DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, const UserHeldTransaction&)
{
    auto& db = m_dbHandler.GetDatabase();
    std::string keyStr(propertyKey);

    const MetadataEntry& meta = properties.GetMetadataEntry(propertyKey);
    auto preparedStatement = db.prepare(
        update(propertiesTable)
            .where(propertiesTable.deviceId == deviceId.GetValue() && propertiesTable.propertyKey == keyStr)
            .set(propertiesTable.propertyValue = parameter(propertiesTable.propertyValue)));
    const nlohmann::json& value = properties.Get(propertyKey);
    if (value != nullptr)
    {
        google::protobuf::Any any = JsonToAny(value);
        std::vector<uint8_t> data(any.ByteSize());
        any.SerializeToArray(data.data(), data.size());
        preparedStatement.params.propertyValue = data;
    }
    else
    {
        preparedStatement.params.propertyValue.set_null();
    }
    db(preparedStatement);
}

void DBDeviceSerialize::InsertDeviceProperty(
    DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    InsertDeviceProperty(deviceId, propertyKey, properties, {user, transaction});
    transaction.commit();
}

void DBDeviceSerialize::InsertDeviceProperty(
    DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, const UserHeldTransaction&)
{
    auto& db = m_dbHandler.GetDatabase();
    std::string keyStr(propertyKey);

    const MetadataEntry& meta = properties.GetMetadataEntry(propertyKey);
    auto preparedStatement
        = db.prepare(insert_into(propertiesTable)
                         .set(propertiesTable.deviceId = deviceId.GetValue(), propertiesTable.propertyKey = keyStr,
                             propertiesTable.propertyValue = parameter(propertiesTable.propertyValue)));
    const nlohmann::json& value = properties.Get(propertyKey);
    if (value != nullptr)
    {
        google::protobuf::Any any = JsonToAny(value);
        std::vector<uint8_t> data(any.ByteSize());
        any.SerializeToArray(data.data(), data.size());
        preparedStatement.params.propertyValue = data;
    }
    else
    {
        preparedStatement.params.propertyValue.set_null();
    }
    // The following does not work correctly
    // if (db(preparedStatement) != 1)
    // {
    //     Res::Logger().Warning(
    //         "DBDeviceSerialize", "InsertDeviceProperty could not insert property \"" + std::string(propertyKey) +
    //         "\"");
    // }
}

constexpr PropertiesLogTable properties;
using Result = decltype(GetSelectResult(properties, properties.propertyValue, properties.propertyDate));

void averageAndSetValue(std::vector<std::pair<std::time_t, double>>& values, nlohmann::json& json)
{
    double averageVal = 0.0;
    std::time_t averageTime = 0;
    for (const auto& point : values)
    {
        averageTime += point.first;
        averageVal += point.second;
    }
    const size_t size = values.size();
    averageTime /= size;
    averageVal /= size;
    averageVal = round(averageVal * 100) / 100;
    json[std::ctime(&averageTime)] = averageVal;

    values.clear();
}

nlohmann::json handleGetPropertyHistory(Result& result, std::time_t compression)
{
    nlohmann::json dataJson = {};
    std::vector<std::pair<std::time_t, double>> avgValues {};
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
                throw std::runtime_error("DBDeviceSerialize::GetPropertyHistory: Invalid blob data");
            }
            value = UnpackAny(any);
        }

        if (value.is_number())
        {
            std::time_t time = std::chrono::system_clock::to_time_t(row.propertyDate.value());
            if (!avgValues.empty() && time - avgValues[0].first > compression)
            {
                averageAndSetValue(avgValues, dataJson);
            }
            avgValues.push_back({time, value});
        }
        else
        {
            std::time_t time = std::chrono::system_clock::to_time_t(row.propertyDate.value());
            // propertyJson[std::ctime(&time)] = value;
            dataJson[std::ctime(&time)] = value;
            // 2007-08-31T16:47+00:00
            // %Y-%m-%d = %F
            // %FT%T%Ez
            // propertyJson[date::format("%FT%T%Ez",
            //     date::zoned_time<std::chrono::system_clock::duration> {"Etc/UTC", row.propertyDate.value()})]
            //     = value;}
        }
    }

    if (!avgValues.empty())
    {
        averageAndSetValue(avgValues, dataJson);
    }

    return dataJson;
}

nlohmann::json DBDeviceSerialize::GetPropertyHistory(DeviceId deviceId, absl::string_view propertyKey,
    const std::chrono::system_clock::time_point& start, absl::optional<const std::chrono::system_clock::time_point> end,
    std::time_t compression, const Properties& properties, UserId user)
{
    auto& db = m_dbHandler.GetDatabase();

    const int64_t devId = deviceId.GetValue();
    const std::string propertyStr = std::string(propertyKey);
    nlohmann::json dataJson;
    // nlohmann::json& propertyJson = dataJson[propertyStr];
    if (end.has_value())
    {
        Result result
            = db(select(propertiesLogTable.propertyValue, propertiesLogTable.propertyDate)
                     .from(propertiesLogTable)
                     .where(propertiesLogTable.deviceId == devId && propertiesLogTable.propertyKey == propertyStr
                         && propertiesLogTable.propertyDate >= start && propertiesLogTable.propertyDate <= end.value())
                     .order_by(propertiesLogTable.propertyDate.asc()));

        dataJson = handleGetPropertyHistory(result, compression);
    }
    else
    {
        Result result
            = db(select(propertiesLogTable.propertyValue, propertiesLogTable.propertyDate)
                     .from(propertiesLogTable)
                     .where(propertiesLogTable.deviceId == devId && propertiesLogTable.propertyKey == propertyStr
                         && propertiesLogTable.propertyDate >= start)
                     .order_by(propertiesLogTable.propertyDate.asc()));

        dataJson = handleGetPropertyHistory(result, compression);
    }
    return dataJson;
}

void DBDeviceSerialize::LogDeviceProperty(
    DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    LogDeviceProperty(deviceId, propertyKey, properties, {user, transaction});
    transaction.commit();
}

void DBDeviceSerialize::LogDeviceProperty(
    DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, const UserHeldTransaction&)
{
    auto& db = m_dbHandler.GetDatabase();
    std::string keyStr(propertyKey);

    const MetadataEntry& meta = properties.GetMetadataEntry(propertyKey);
    auto preparedStatement = db.prepare(
        insert_into(propertiesLogTable)
            .set(propertiesLogTable.deviceId = deviceId.GetValue(), propertiesLogTable.propertyKey = keyStr,
                propertiesLogTable.propertyValue = parameter(propertiesLogTable.propertyValue),
                propertiesLogTable.propertyDate = parameter(propertiesLogTable.propertyDate)));
    // preparedStatement.params.propertyDate
    //     = date::zoned_time<std::chrono::system_clock::duration> {std::chrono::system_clock::now()};

    preparedStatement.params.propertyDate
        = std::chrono::time_point_cast<sqlpp::chrono::microsecond_point::duration>(std::chrono::system_clock::now());
    const nlohmann::json& value = properties.Get(propertyKey);
    if (value != nullptr)
    {
        google::protobuf::Any any = JsonToAny(value);
        std::vector<uint8_t> data(any.ByteSize());
        any.SerializeToArray(data.data(), data.size());
        preparedStatement.params.propertyValue = data;
    }
    else
    {
        preparedStatement.params.propertyValue.set_null();
    }
    db(preparedStatement);
}

void DBDeviceSerialize::InsertDeviceGroups(
    DeviceId id, const std::vector<std::string>& groups, const UserHeldTransaction&)
{
    auto& db = m_dbHandler.GetDatabase();
    auto preparedStatement = db.prepare(insert_into(deviceGroups)
                                            .set(deviceGroups.deviceId = id.GetValue(),
                                                deviceGroups.groupName = sqlpp::parameter(deviceGroups.groupName)));
    for (const std::string& group : groups)
    {
        preparedStatement.params.groupName = group;
        db(preparedStatement);
    }
}

void DBDeviceSerialize::AddProperties(DeviceId deviceId, const Properties& properties, const UserHeldTransaction&)
{
    auto& db = m_dbHandler.GetDatabase();
    auto preparedStatement
        = db.prepare(insert_into(propertiesTable)
                         .set(propertiesTable.deviceId = deviceId.GetValue(),
                             propertiesTable.propertyKey = parameter(propertiesTable.propertyKey),
                             propertiesTable.propertyValue = parameter(propertiesTable.propertyValue)));
    for (const auto& p : properties.GetAll())
    {
        preparedStatement.params.propertyKey = p.first;
        if (p.second != nullptr)
        {
            google::protobuf::Any any = JsonToAny(p.second);
            std::vector<uint8_t> data(any.ByteSize());
            any.SerializeToArray(data.data(), data.size());
            preparedStatement.params.propertyValue = data;
        }
        else
        {
            preparedStatement.params.propertyValue.set_null();
        }
        db(preparedStatement);
    }
}

std::vector<std::string> DBDeviceSerialize::GetDeviceGroups(DeviceId deviceId, const UserHeldTransaction&) const
{
    auto& db = m_dbHandler.GetDatabase();
    auto result
        = db(select(deviceGroups.groupName).from(deviceGroups).where(deviceGroups.deviceId == deviceId.GetValue()));
    std::vector<std::string> groups;
    for (const auto& row : result)
    {
        groups.push_back(row.groupName);
    }
    return groups;
}

Properties DBDeviceSerialize::GetDeviceProperties(
    DeviceId deviceId, const DeviceType& type, const UserHeldTransaction&) const
{
    auto& db = m_dbHandler.GetDatabase();
    auto result = db(select(propertiesTable.propertyKey, propertiesTable.propertyValue)
                         .from(propertiesTable)
                         .where(propertiesTable.deviceId == deviceId.GetValue()));
    absl::flat_hash_map<std::string, nlohmann::json> values;
    for (const auto& row : result)
    {
        nlohmann::json json;
        if (row.propertyValue.is_null())
        {
            json = nullptr;
        }
        else
        {
            google::protobuf::Any any;
            if (!any.ParseFromArray(row.propertyValue.blob, row.propertyValue.len))
            {
                throw std::runtime_error("DBDeviceSerialize::GetDeviceProperties: Invalid blob data");
            }
            json = UnpackAny(any);
        }
        values.emplace(row.propertyKey, std::move(json));
    }

    return Properties::FromRawData(std::move(values), type);
}
