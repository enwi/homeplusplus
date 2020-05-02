#include "RemoteSocketType.h"

RemoteSocketType::RemoteSocketType(RemoteSocketTransmitter& transmitter)
    :m_transmitter(transmitter)
{
    MetadataEntry stateMeta = MetadataEntry::Builder()
                                  .SetType(MetadataEntry::DataType::boolean)
                                  .SetSave(MetadataEntry::DBSave::save_log)
                                  .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead
                                      | MetadataEntry::Access::userWrite | MetadataEntry::Access::actionWrite)
                                  .Create();
    MetadataEntry codeMeta = MetadataEntry::Builder()
                                 .SetType(MetadataEntry::DataType::string)
                                 .SetSave(MetadataEntry::DBSave::save)
                                 .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead
                                     | MetadataEntry::Access::userWrite | MetadataEntry::Access::actionWrite)
                                 .Create();
    absl::flat_hash_map<std::string, MetadataEntry> entries{{"state", stateMeta}, {"code", codeMeta}};
    m_meta = Metadata(std::move(entries));
}

bool RemoteSocketType::ValidateUpdate(absl::string_view property, const nlohmann::json& value, UserId user) const
{
    if (property == "state")
    {
        return true;
    }
    else if (property == "code")
    {
        std::string code = value;
        // TODO: validate code
        return true;
    }
    return false;
}

void RemoteSocketType::OnUpdate(absl::string_view property, Device& device, UserId user) const
{
    if (property == "state")
    {
        m_transmitter.SetSocket(device.GetProperty("code"), device.GetProperty("state"));
    }
}