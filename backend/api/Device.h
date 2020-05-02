#pragma once

#include <type_traits>

#include <absl/container/flat_hash_map.h>
#include <absl/container/inlined_vector.h>
#include <absl/strings/string_view.h>
#include <absl/types/optional.h>
#include <google/protobuf/map.h>
#include <json.hpp>

#include "User.h"

#include "../events/EventSystem.h"
#include "api/device.pb.h"

class DeviceId
{
public:
    explicit DeviceId(int64_t value) : m_value(value) {}
    friend bool operator==(DeviceId l, DeviceId r) { return l.m_value == r.m_value; }
    template <typename H>
    friend H AbslHashValue(H h, const DeviceId& d)
    {
        return H::combine(std::move(h), d.m_value);
    }
    int64_t GetValue() const { return m_value; }

private:
    int64_t m_value = 0;
};

class MetadataEntry
{
public:
    enum class DataType
    {
        none,
        boolean,
        integer,
        floatingPoint,
        string,
        // Custom: custom protobuf message
        custom
    };
    enum class DBSave
    {
        none,
        save,
        save_log
    };
    enum class Access : int
    {
        none = 0,
        userRead = 1,
        userWrite = 2,
        ruleRead = 4,
        actionWrite = 8
    };
    enum class FetchFrequency
    {
        none
    };

    class Builder
    {
    public:
        Builder() = default;

        MetadataEntry Create() { return MetadataEntry(m_type, m_save, m_access, m_frequency, m_isOptional); }
        Builder& SetType(DataType type)
        {
            m_type = type;
            return *this;
        }
        Builder& SetSave(DBSave save)
        {
            m_save = save;
            return *this;
        }
        Builder& SetAccess(Access access)
        {
            m_access = access;
            return *this;
        }
        Builder& SetFetchFrequency(FetchFrequency frequency)
        {
            m_frequency = frequency;
            return *this;
        }
        Builder& SetOptional(bool optional)
        {
            m_isOptional = optional;
            return *this;
        }

    private:
        DBSave m_save = DBSave::none;
        Access m_access = Access::none;
        FetchFrequency m_frequency = FetchFrequency::none;
        DataType m_type = DataType::none;
        bool m_isOptional = false;
    };

public:
    MetadataEntry(DataType type, DBSave save, Access access, FetchFrequency frequency, bool optional)
        : m_save(save), m_access(access), m_frequency(frequency), m_type(type), m_isOptional(optional)
    {}

    DBSave GetDBSave() const { return m_save; }
    Access GetAccess() const { return m_access; }
    FetchFrequency GetFetchFrequency() const { return m_frequency; }
    DataType GetType() const { return m_type; }
    bool IsOptional() const { return m_isOptional; }

    bool ValidateType(const nlohmann::json& value) const;

private:
    DBSave m_save;
    Access m_access;
    FetchFrequency m_frequency;
    DataType m_type;
    bool m_isOptional;
};

inline MetadataEntry::Access operator|(MetadataEntry::Access lhs, MetadataEntry::Access rhs)
{
    using T = std::underlying_type_t<MetadataEntry::Access>;
    return static_cast<MetadataEntry::Access>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline MetadataEntry::Access operator&(MetadataEntry::Access lhs, MetadataEntry::Access rhs)
{
    using T = std::underlying_type_t<MetadataEntry::Access>;
    return static_cast<MetadataEntry::Access>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

class Metadata
{
public:
    Metadata() = default;
    explicit Metadata(absl::flat_hash_map<std::string, MetadataEntry> entries) : m_entries(std::move(entries)) {}

    const MetadataEntry& GetEntry(absl::string_view path) const;
    MetadataEntry& GetEntry(absl::string_view path);
    bool HasEntry(absl::string_view path) const;

    std::vector<std::string> GetEntryPaths() const;

private:
    absl::flat_hash_map<std::string, MetadataEntry> m_entries;
};

class Properties
{
public:
    bool Set(absl::string_view path, const nlohmann::json& value, class Device& device, class DeviceStorage& storage,
        UserId user);
    nlohmann::json Get(absl::string_view path) const;
    nlohmann::json GetHistory(DeviceId deviceId, absl::string_view path,
        const std::chrono::system_clock::time_point& start,
        absl::optional<const std::chrono::system_clock::time_point> end, std::time_t compression,
        DeviceStorage& storage, UserId user) const;
    const Metadata& GetMetadata() const;
    const MetadataEntry& GetMetadataEntry(absl::string_view path) const;

    google::protobuf::Map<std::string, google::protobuf::Any> Serialize() const;
    void Deserialize(const google::protobuf::Map<std::string, google::protobuf::Any>& msg);
    nlohmann::json ToJson() const;

    const absl::flat_hash_map<std::string, nlohmann::json>& GetAll() const;
    // Does not perform any checks
    static Properties FromRawData(
        absl::flat_hash_map<std::string, nlohmann::json> values, const class DeviceType& type);

private:
    bool CheckPermissions(absl::string_view path, const MetadataEntry& meta, UserId user);
    bool Validate(absl::string_view path, const MetadataEntry& meta, const nlohmann::json& value, UserId user);
    void SaveDatabase(absl::string_view path, const MetadataEntry& meta, const nlohmann::json& value, DeviceId id,
        class DeviceStorage& storage, UserId user);
    void InsertDatabase(absl::string_view path, const MetadataEntry& meta, const nlohmann::json& value, DeviceId id,
        DeviceStorage& storage, UserId user);
    nlohmann::json GetDatabaseHistory(DeviceId id, absl::string_view path,
        const std::chrono::system_clock::time_point& start,
        absl::optional<const std::chrono::system_clock::time_point> end, std::time_t compression,
        DeviceStorage& storage, UserId user) const;

private:
    const class DeviceType* m_type;
    absl::flat_hash_map<std::string, nlohmann::json> m_values;
};

class Device
{
public:
    friend class DeviceStorage;
    struct Data
    {
        std::string m_name;
        std::string m_icon;
        std::vector<std::string> m_groups;
        std::string m_type;
        Properties m_properties;
        std::string m_api;
    };

public:
    Device() : m_data(std::make_shared<Data>()), m_id(0) {}
    Device(absl::string_view name, absl::string_view icon, std::vector<std::string> groups, absl::string_view type,
        Properties properties, absl::string_view api)
        : m_data(std::make_shared<Data>(Data {std::string(name), std::string(icon), std::move(groups),
            std::string(type), std::move(properties), std::string(api)})),
          m_id(0)
    {}
    std::string GetName() const;
    void SetName(absl::string_view name);

    std::string GetIcon() const;
    void SetIcon(absl::string_view name);

    const std::vector<std::string>& GetGroups() const;
    void SetGroups(const std::vector<std::string>& groups);

    std::string GetType() const;

    DeviceId GetId() const;
    Properties& GetProperties();
    const Properties& GetProperties() const;
    bool SetProperty(absl::string_view path, const nlohmann::json& value, class DeviceStorage& storage, UserId user);
    nlohmann::json GetProperty(absl::string_view path) const;
    nlohmann::json GetPropertyHistory(absl::string_view path, const std::chrono::system_clock::time_point& start,
        absl::optional<const std::chrono::system_clock::time_point> end, std::time_t compression,
        DeviceStorage& storage, UserId user) const;

    messages::Device Serialize() const;
    // Updates shared data of device
    void Deserialize(const messages::Device& msg);
    nlohmann::json ToJson() const;

private:
    Device(DeviceId id, std::shared_ptr<Data> data);

private:
    DeviceId m_id;
    // Data is shared between all instances of one device
    std::shared_ptr<Data> m_data;
};
