#include "Device.h"

#include <google/protobuf/wrappers.pb.h>

#include "DeviceStorage.h"
#include "DeviceType.h"

bool MetadataEntry::ValidateType(const nlohmann::json& value) const
{
    if (m_isOptional && value.is_null())
    {
        return true;
    }
    switch (m_type)
    {
    case DataType::none:
        return value.is_null();
    case DataType::boolean:
        return value.is_boolean();
    case DataType::integer:
        return value.is_number_integer();
    case DataType::floatingPoint:
        return value.is_number_float();
    case DataType::string:
        return value.is_string();
    case DataType::custom:
        return true;
    default:
        return false;
    }
}

const MetadataEntry& Metadata::GetEntry(absl::string_view path) const
{
    return m_entries.at(path);
}

MetadataEntry& Metadata::GetEntry(absl::string_view path)
{
    return m_entries.at(path);
}

bool Metadata::HasEntry(absl::string_view path) const
{
    return m_entries.count(path) > 0;
}

std::vector<std::string> Metadata::GetEntryPaths() const
{
    std::vector<std::string> result;
    result.reserve(m_entries.size());
    for (const auto& pair : m_entries)
    {
        result.push_back(pair.first);
    }
    return result;
}

bool Properties::Set(
    absl::string_view path, const nlohmann::json& value, Device& device, DeviceStorage& storage, UserId user)
{
    if (GetMetadata().HasEntry(path))
    {
        const MetadataEntry& meta = GetMetadata().GetEntry(path);
        if (CheckPermissions(path, meta, user) && Validate(path, meta, value, user))
        {
            if (m_values.count(path))
            {
                m_values[path] = value;
                SaveDatabase(path, meta, value, device.GetId(), storage, user);
            }
            else
            {
                Res::Logger().Debug("Properties", "Creating missing property \"" + std::string(path) + "\"");
                m_values[path] = value;
                InsertDatabase(path, meta, value, device.GetId(), storage, user);
            }
            m_type->OnUpdate(path, device, user);
            return true;
        }
    }
    return false;
}

nlohmann::json Properties::Get(absl::string_view path) const
{
    if (GetMetadata().HasEntry(path))
    {
        return m_values.at(path);
    }
    return nullptr;
}

nlohmann::json Properties::GetHistory(DeviceId deviceId, absl::string_view path,
    const std::chrono::system_clock::time_point& start, absl::optional<const std::chrono::system_clock::time_point> end,
    std::time_t compression, DeviceStorage& storage, UserId user) const
{
    if (GetMetadata().HasEntry(path))
    {
        // const MetadataEntry& meta = GetMetadata().GetEntry(path);
        // if (CheckPermissions(path, meta, user))
        // {
        return GetDatabaseHistory(deviceId, path, start, end, compression, storage, user);
        // }
    }
    return nullptr;
}

const Metadata& Properties::GetMetadata() const
{
    return m_type->GetDeviceMetadata();
}

const MetadataEntry& Properties::GetMetadataEntry(absl::string_view path) const
{
    return GetMetadata().GetEntry(path);
}

google::protobuf::Map<std::string, google::protobuf::Any> Properties::Serialize() const
{
    google::protobuf::Map<std::string, google::protobuf::Any> map;
    for (const std::pair<std::string, nlohmann::json> v : m_values)
    {
        google::protobuf::MapPair<std::string, google::protobuf::Any> pair(v.first, google::protobuf::Any());
        google::protobuf::StringValue sv;
        sv.set_value(v.second.dump());
        pair.second.PackFrom(sv);
        // TODO: Convert json to some binary form
        map.insert(pair);
    }
    return map;
}

void Properties::Deserialize(const google::protobuf::Map<std::string, google::protobuf::Any>& msg)
{
    // Create another map to guarantee exception safety
    absl::flat_hash_map<std::string, nlohmann::json> values;
    for (const auto& p : msg)
    {
        google::protobuf::StringValue sv;
        if (!p.second.UnpackTo(&sv))
        {
            throw std::invalid_argument("Properties::Deserialize");
        }
        nlohmann::json json = nlohmann::json::parse(sv.value());
        values.emplace(p.first, std::move(json));
    }
    m_values = std::move(values);
}

nlohmann::json Properties::ToJson() const
{
    nlohmann::json json;
    for (const auto& p : m_values)
    {
        json[p.first] = p.second;
    }
    return json;
}

const absl::flat_hash_map<std::string, nlohmann::json>& Properties::GetAll() const
{
    return m_values;
}

Properties Properties::FromRawData(absl::flat_hash_map<std::string, nlohmann::json> values, const DeviceType& type)
{
    Properties p;
    p.m_values = std::move(values);
    p.m_type = &type;
    return p;
}

bool Properties::CheckPermissions(absl::string_view path, const MetadataEntry& meta, UserId user)
{
    // TODO: Implement permission check.
    return true;
}

bool Properties::Validate(absl::string_view path, const MetadataEntry& meta, const nlohmann::json& value, UserId user)
{
    if (!meta.ValidateType(value))
    {
        Res::Logger().Debug("Property", "Validate for " + std::string(path) + " failed, invalid type");
        return false;
    }
    if (!m_type->ValidateUpdate(path, value, user))
    {
        Res::Logger().Debug("Property", "Validator for " + std::string(path) + " failed");
        return false;
    }
    return true;
}

void Properties::SaveDatabase(absl::string_view path, const MetadataEntry& meta, const nlohmann::json& value,
    DeviceId id, DeviceStorage& storage, UserId user)
{
    const MetadataEntry::DBSave dbSave = meta.GetDBSave();
    if (dbSave == MetadataEntry::DBSave::save)
    {
        storage.SetDeviceProperty(id, path, *this, user);
    }
    else if (dbSave == MetadataEntry::DBSave::save_log)
    {
        storage.SetAndLogDeviceProperty(id, path, *this, user);
    }
}

void Properties::InsertDatabase(absl::string_view path, const MetadataEntry& meta, const nlohmann::json& value,
    DeviceId id, DeviceStorage& storage, UserId user)
{
    const MetadataEntry::DBSave dbSave = meta.GetDBSave();
    if (dbSave == MetadataEntry::DBSave::save)
    {
        storage.InsertDeviceProperty(id, path, *this, user);
    }
    else if (dbSave == MetadataEntry::DBSave::save_log)
    {
        storage.InsertAndLogDeviceProperty(id, path, *this, user);
    }
}

nlohmann::json Properties::GetDatabaseHistory(DeviceId id, absl::string_view path,
    const std::chrono::system_clock::time_point& start, absl::optional<const std::chrono::system_clock::time_point> end,
    std::time_t compression, DeviceStorage& storage, UserId user) const
{
    // TODO do we need to differentiate between MetadataEntry::DBSave::save and MetadataEntry::DBSave::save_log?
    return storage.GetPropertyHistory(id, path, start, end, compression, *this, user);
}

std::string Device::GetName() const
{
    return m_data->m_name;
}

void Device::SetName(absl::string_view name)
{
    m_data->m_name = std::string(name);
}

std::string Device::GetIcon() const
{
    return m_data->m_icon;
}

void Device::SetIcon(absl::string_view name)
{
    m_data->m_icon = std::string(name);
}

const std::vector<std::string>& Device::GetGroups() const
{
    return m_data->m_groups;
}

void Device::SetGroups(const std::vector<std::string>& groups)
{
    m_data->m_groups = groups;
}

std::string Device::GetType() const
{
    return m_data->m_type;
}

DeviceId Device::GetId() const
{
    return m_id;
}

Properties& Device::GetProperties()
{
    return m_data->m_properties;
}

const Properties& Device::GetProperties() const
{
    return m_data->m_properties;
}

bool Device::SetProperty(absl::string_view path, const nlohmann::json& value, DeviceStorage& storage, UserId user)
{
    return GetProperties().Set(path, value, *this, storage, user);
}

nlohmann::json Device::GetProperty(absl::string_view path) const
{
    return GetProperties().Get(path);
}

nlohmann::json Device::GetPropertyHistory(absl::string_view path, const std::chrono::system_clock::time_point& start,
    absl::optional<const std::chrono::system_clock::time_point> end, std::time_t compression, DeviceStorage& storage,
    UserId user) const
{
    return GetProperties().GetHistory(m_id, path, start, end, compression, storage, user);
}

Device::Device(DeviceId id, std::shared_ptr<Data> data) : m_id(id), m_data(std::move(data))
{
    assert(m_data != nullptr);
}

messages::Device Device::Serialize() const
{
    messages::Device msg;
    msg.set_id(m_id.GetValue());
    for (const std::string& group : m_data->m_groups)
    {
        *msg.add_groups() = group;
    }
    msg.set_name(m_data->m_name);
    msg.set_icon(m_data->m_icon);
    *msg.mutable_properties() = m_data->m_properties.Serialize();
    return msg;
}

void Device::Deserialize(const messages::Device& msg)
{
    m_data->m_properties.Deserialize(msg.properties());
    for (const std::string& group : msg.groups())
    {
        m_data->m_groups.push_back(group);
    }
    m_data->m_name = msg.name();
    m_data->m_icon = msg.icon();
    m_id = DeviceId(msg.id());
}

nlohmann::json Device::ToJson() const
{
    nlohmann::json groups(nlohmann::json::array());
    for (const std::string& g : m_data->m_groups)
    {
        groups.push_back(g);
    }
    return {{"id", m_id.GetValue()}, {"groups", groups}, {"name", m_data->m_name}, {"icon", m_data->m_icon},
        {"type", m_data->m_type}, {"properties", m_data->m_properties.ToJson()}};
}