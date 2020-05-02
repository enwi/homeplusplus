#include "DeviceType.h"

void DeviceTypeRegistry::AddDeviceType(std::unique_ptr<DeviceType> type)
{
    if (type == nullptr)
    {
        throw std::invalid_argument("Device type is null");
    }
    absl::string_view name = type->GetName();
    m_types.insert_or_assign(name, std::move(type));
}

const DeviceType& DeviceTypeRegistry::GetDeviceType(absl::string_view type) const
{
    return *m_types.at(type);
}

bool DeviceTypeRegistry::HasDeviceType(absl::string_view type) const
{
    return m_types.count(type) != 0;
}

std::vector<std::string> DeviceTypeRegistry::GetRegisteredTypes() const
{
    std::vector<std::string> result;
    result.reserve(m_types.size());
    for (const auto& pair : m_types)
    {
        result.push_back(std::string(pair.first));
    }
    return result;
}
