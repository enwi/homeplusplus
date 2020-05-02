#include "DeviceStorage.h"

#include "IDeviceSerialize.h"

DeviceStorage::DeviceStorage(IDeviceSerialize& deviceSer, EventEmitter<Events::DeviceChangeEvent>& eventEmitter,
    EventEmitter<Events::DevicePropertyChangeEvent>& propertyEvents)
    : m_serialize(&deviceSer), m_eventEmitter(&eventEmitter), m_propertyEvents(&propertyEvents)
{}

DeviceId DeviceStorage::AddDevice(const Device& d, UserId u)
{
    DeviceId id = m_serialize->AddDevice(*d.m_data, u);
    m_cache.insert_or_assign(id, d.m_data);
    m_eventEmitter->EmitEvent(Events::DeviceChangeEvent(Device(), d, Events::DeviceFields::ADD, u));
    return id;
}

void DeviceStorage::RemoveDevice(DeviceId id, UserId u)
{
    absl::optional<Device> d = GetDevice(id, u);
    m_cache.erase(id);
    m_serialize->RemoveDevice(id, u);
    if (d)
    {
        m_eventEmitter->EmitEvent(Events::DeviceChangeEvent(*d, Device(), Events::DeviceFields::REMOVE, u));
    }
}

void DeviceStorage::ChangeDevice(const Device& d, UserId u)
{
    // TODO: Find other way to specify old value, this does not work
    absl::optional<Device> old = GetDevice(d.GetId(), u);
    m_serialize->UpdateDevice(d.GetId(), *d.m_data, u);
    m_cache.insert_or_assign(d.GetId(), d.m_data);
    if (old)
    {
        m_eventEmitter->EmitEvent(Events::DeviceChangeEvent(*old, d, Events::DeviceFields::ALL, u));
    }
    else
    {
        m_eventEmitter->EmitEvent(Events::DeviceChangeEvent(Device(), d, Events::DeviceFields::ALL, u));
    }
}

absl::optional<Device> DeviceStorage::GetDevice(DeviceId id, UserId u)
{
    auto it = m_cache.find(id);
    if (it != m_cache.end())
    {
        if (std::shared_ptr<Device::Data> dataPtr = it->second.lock())
        {
            return Device(id, dataPtr);
        }
    }
    auto deviceData = m_serialize->GetDeviceData(id, u);
    if (deviceData)
    {
        auto dataPtr = std::make_shared<Device::Data>(*deviceData);
        m_cache.insert_or_assign(id, dataPtr);
        return Device(id, dataPtr);
    }
    return absl::nullopt;
}

std::vector<Device> DeviceStorage::GetApiDevices(absl::string_view apiId, UserId u)
{
    std::vector<DeviceId> deviceIds = m_serialize->GetAPIDeviceIds(apiId, u);
    std::vector<Device> result;
    result.reserve(deviceIds.size());
    for (DeviceId id : deviceIds)
    {
        absl::optional<Device> device = GetDevice(id, u);
        if (device)
        {
            result.emplace_back(std::move(device).value());
        }
    }
    return result;
}

std::vector<Device> DeviceStorage::GetAllDevices(UserId u)
{
    std::vector<DeviceId> deviceIds = m_serialize->GetAllDeviceIds(Filter(), u);
    std::vector<Device> result;
    result.reserve(deviceIds.size());
    for (DeviceId id : deviceIds)
    {
        absl::optional<Device> device = GetDevice(id, u);
        if (device)
        {
            result.emplace_back(std::move(device).value());
        }
    }
    return result;
}

void DeviceStorage::SetDeviceProperty(DeviceId id, absl::string_view path, const Properties& properties, UserId user)
{
    absl::optional<Device> device = GetDevice(id, user);
    if (device)
    {
        m_serialize->SetDeviceProperty(id, path, properties, user);
        // TODO: Find other way to specify old value, this does not work
        m_propertyEvents->EmitEvent(Events::DevicePropertyChangeEvent(*device, *device, std::string(path), user));
    }
}

void DeviceStorage::SetAndLogDeviceProperty(
    DeviceId id, absl::string_view path, const Properties& properties, UserId user)
{
    absl::optional<Device> device = GetDevice(id, user);
    if (device)
    {
        m_serialize->SetDeviceProperty(id, path, properties, user);
        m_serialize->LogDeviceProperty(id, path, properties, user);
        // TODO: Find other way to specify old value, this does not work
        m_propertyEvents->EmitEvent(Events::DevicePropertyChangeEvent(*device, *device, std::string(path), user));
    }
}

void DeviceStorage::InsertDeviceProperty(DeviceId id, absl::string_view path, const Properties& properties, UserId user)
{
    absl::optional<Device> device = GetDevice(id, user);
    if (device)
    {
        m_serialize->InsertDeviceProperty(id, path, properties, user);
        // TODO: Find other way to specify old value, this does not work
        m_propertyEvents->EmitEvent(Events::DevicePropertyChangeEvent(*device, *device, std::string(path), user));
    }
}

void DeviceStorage::InsertAndLogDeviceProperty(
    DeviceId id, absl::string_view path, const Properties& properties, UserId user)
{
    absl::optional<Device> device = GetDevice(id, user);
    if (device)
    {
        m_serialize->InsertDeviceProperty(id, path, properties, user);
        m_serialize->LogDeviceProperty(id, path, properties, user);
        // TODO: Find other way to specify old value, this does not work
        m_propertyEvents->EmitEvent(Events::DevicePropertyChangeEvent(*device, *device, std::string(path), user));
    }
}

nlohmann::json DeviceStorage::GetPropertyHistory(DeviceId id, absl::string_view path,
    const std::chrono::system_clock::time_point& start, absl::optional<const std::chrono::system_clock::time_point> end,
    std::time_t compression, const Properties& properties, UserId user)
{
    absl::optional<Device> device = GetDevice(id, user);
    if (device)
    {
        return m_serialize->GetPropertyHistory(id, path, start, end, compression, properties, user);
    }
    else
    {
        return nullptr;
    }
}

void DeviceStorage::CleanupCache()
{
    for (auto it = m_cache.begin(), end = m_cache.end(); it != end;)
    {
        if (it->second.expired())
        {
            m_cache.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}