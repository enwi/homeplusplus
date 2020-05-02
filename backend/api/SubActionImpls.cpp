#include "SubActionImpls.h"

#include <thread>

#include "ActionStorage.h"
#include "DeviceRegistry.h"

#include "../communication/WebsocketCommunication.h"
#include "../database/DBActionSerialize.h"
#include "../database/DBHandler.h"
#include "../utility/AnyJson.h"
#include "api/sub_actions.pb.h"

nlohmann::json SubActionImpls::DeviceSet::ToJSON() const
{
    return {{"type", m_type}, {"deviceId", m_deviceId.GetValue()}, {"property", m_property}, {"value", m_value},
        {"timeout", std::chrono::duration_cast<std::chrono::milliseconds>(m_timeout).count()},
        {"transition", m_transition}};
}

messages::SubAction SubActionImpls::DeviceSet::Serialize() const
{
    messages::SubAction msg;
    msg.set_type(m_type);
    msg.set_timeout(std::chrono::duration_cast<std::chrono::milliseconds>(m_timeout).count());
    msg.set_transition(m_transition);
    messages::sub_actions::DeviceSetData data;
    data.set_device_id(m_deviceId.GetValue());
    data.set_property(m_property);
    if (!m_value.is_null())
    {
        *data.mutable_value() = JsonToAny(m_value);
    }
    msg.mutable_data()->PackFrom(data);
    return msg;
}

void SubActionImpls::DeviceSet::Parse(const nlohmann::json& json)
{
    m_type = json.at("type");
    m_deviceId = DeviceId(json.at("deviceId").get<int64_t>());
    m_property = json.at("property").get<std::string>();
    m_value = json.at("value");
    m_timeout = std::chrono::milliseconds(json.at("timeout"));
    m_transition = json.at("transition");
}

void SubActionImpls::DeviceSet::Deserialize(const messages::SubAction& msg)
{
    m_type = msg.type();
    m_timeout = std::chrono::milliseconds(msg.timeout());
    m_transition = msg.transition();
    messages::sub_actions::DeviceSetData data;
    if (!msg.data().UnpackTo(&data))
    {
        throw std::invalid_argument("SubActionImpls::DeviceSet::Deserialize");
    }
    m_deviceId = DeviceId(data.device_id());
    m_property = data.property();
    m_value = UnpackAny(data.value());
}

void SubActionImpls::DeviceSet::Parse(
    DBHandler::DatabaseConnection&, const SubActionsRow& result, const UserHeldTransaction&)
{
    m_type = result.actionType;
    google::protobuf::Any any;
    messages::sub_actions::DeviceSetData data;
    if (result.data.is_null() || !any.ParseFromArray(result.data.blob, result.data.len) || !any.UnpackTo(&data))
    {
        throw std::invalid_argument("SubActionImpls::DeviceSet::Parse(db)");
    }
    m_deviceId = DeviceId(data.device_id());
    m_property = data.property();
    if (data.has_value())
    {
        m_value = UnpackAny(data.value());
    }
    else
    {
        m_value = nullptr;
    }
    m_timeout = std::chrono::milliseconds(result.timeout);
    m_transition = result.transition;
}

void SubActionImpls::DeviceSet::InternalExec(
    ActionStorage& actionStorage, WebsocketChannel&, DeviceRegistry& deviceReg, UserId user, int) const
{
    absl::optional<Device> device = deviceReg.GetStorage().GetDevice(m_deviceId, user);
    if (!device)
    {
        throw std::runtime_error(
            "Tried to set a property of a nonexistent device. ID: " + std::to_string(m_deviceId.GetValue()));
    }
    Properties& properties = device->GetProperties();
    if ((properties.GetMetadataEntry(m_property).GetAccess() & MetadataEntry::Access::actionWrite)
            == MetadataEntry::Access::none
        || !properties.Set(m_property, m_value, *device, deviceReg.GetStorage(), user))
    {
        throw std::runtime_error("Device property set failed");
    }
}

SubActionImpls::DeviceSet SubActionImpls::DeviceSet::Create(uint64_t type, DeviceId deviceId,
    absl::string_view property, const std::string& value, std::chrono::steady_clock::duration timeout, bool transition)
{
    DeviceSet a{type};
    a.m_deviceId = deviceId;
    a.m_property = std::string(property);
    a.m_value = value;
    a.m_timeout = timeout;
    a.m_transition = transition;
    return a;
}

nlohmann::json SubActionImpls::DeviceToggle::ToJSON() const
{
    return {{"type", m_type}, {"deviceId", m_deviceId.GetValue()}, {"property", m_property},
        {"timeout", std::chrono::duration_cast<std::chrono::milliseconds>(m_timeout).count()},
        {"transition", m_transition}};
}

messages::SubAction SubActionImpls::DeviceToggle::Serialize() const
{
    messages::SubAction msg;
    msg.set_type(m_type);
    msg.set_timeout(std::chrono::duration_cast<std::chrono::milliseconds>(m_timeout).count());
    msg.set_transition(m_transition);
    messages::sub_actions::DeviceToggleData data;
    data.set_device_id(m_deviceId.GetValue());
    data.set_property(m_property);
    msg.mutable_data()->PackFrom(data);
    return msg;
}

void SubActionImpls::DeviceToggle::Parse(const nlohmann::json& json)
{
    m_type = json.at("type");
    m_deviceId = DeviceId(json.at("deviceId").get<int64_t>());
    m_property = json.at("property").get<std::string>();
    m_timeout = std::chrono::milliseconds(json.at("timeout"));
    m_transition = json.at("transition");
}

void SubActionImpls::DeviceToggle::Deserialize(const messages::SubAction& msg)
{
    m_type = msg.type();
    m_timeout = std::chrono::milliseconds(msg.timeout());
    m_transition = msg.transition();
    messages::sub_actions::DeviceToggleData data;
    if (!msg.data().UnpackTo(&data))
    {
        throw std::invalid_argument("SubActionImpls::DeviceToggle::Deserialize");
    }
    m_deviceId = DeviceId(data.device_id());
    m_property = data.property();
}

void SubActionImpls::DeviceToggle::Parse(
    DBHandler::DatabaseConnection&, const SubActionsRow& result, const UserHeldTransaction&)
{
    m_type = result.actionType;
    google::protobuf::Any any;
    messages::sub_actions::DeviceToggleData data;
    if (result.data.is_null() || !any.ParseFromArray(result.data.blob, result.data.len) || !any.UnpackTo(&data))
    {
        throw std::invalid_argument("SubActionImpls::DeviceToggle::Parse(db)");
    }
    m_deviceId = DeviceId(data.device_id());
    m_property = data.property();
    m_timeout = std::chrono::milliseconds(result.timeout);
    m_transition = result.transition;
}

void SubActionImpls::DeviceToggle::InternalExec(
    ActionStorage& actionStorage, WebsocketChannel&, DeviceRegistry& deviceReg, UserId user, int) const
{
    // TODO: Fix toggle for not 0/1 actors
    absl::optional<Device> device = deviceReg.GetStorage().GetDevice(m_deviceId, user);
    if (!device)
    {
        throw std::runtime_error(
            "Tried to toggle a property of a nonexistent device. ID: " + std::to_string(m_deviceId.GetValue()));
    }
    Properties& properties = device->GetProperties();
    if ((properties.GetMetadataEntry(m_property).GetAccess() & MetadataEntry::Access::actionWrite)
        == MetadataEntry::Access::none)
    {
        throw std::runtime_error("Device property toggle failed");
    }
    nlohmann::json value = properties.Get(m_property);
    if (value.type() == nlohmann::json::value_t::boolean)
    {
        properties.Set(m_property, !value.get<bool>(), *device, deviceReg.GetStorage(), user);
    }
    else if (value.type() == nlohmann::json::value_t::number_integer
        || value.type() == nlohmann::json::value_t::number_unsigned)
    {
        properties.Set(m_property, value == 0 ? 1 : 0, *device, deviceReg.GetStorage(), user);
    }
}
SubActionImpls::DeviceToggle SubActionImpls::DeviceToggle::Create(uint64_t type, DeviceId deviceId,
    absl::string_view property, std::chrono::steady_clock::duration timeout, bool transition)
{
    DeviceToggle a{type};
    a.m_deviceId = deviceId;
    a.m_property = std::string(property);
    a.m_timeout = timeout;
    a.m_transition = transition;
    return a;
}

nlohmann::json SubActionImpls::Notification::ToJSON() const
{
    return {{"type", m_type}, {"category", m_category}, {"message", m_message},
        {"timeout", std::chrono::duration_cast<std::chrono::milliseconds>(m_timeout).count()},
        {"transition", m_transition}};
}

messages::SubAction SubActionImpls::Notification::Serialize() const
{
    messages::SubAction msg;
    msg.set_timeout(std::chrono::duration_cast<std::chrono::milliseconds>(m_timeout).count());
    msg.set_transition(m_transition);
    msg.set_type(m_type);
    messages::sub_actions::NotificationData data;
    data.set_category(m_category);
    data.set_message(m_message);
    msg.mutable_data()->PackFrom(data);
    return msg;
}

void SubActionImpls::Notification::Parse(const nlohmann::json& json)
{
    m_type = json.at("type");
    m_category = json.at("category");
    m_message = json.at("message").get<std::string>();
    m_timeout = std::chrono::milliseconds(json.at("timeout"));
    m_transition = json.at("transition");
}

void SubActionImpls::Notification::Deserialize(const messages::SubAction& msg)
{
    m_type = msg.type();
    m_timeout = std::chrono::milliseconds(msg.timeout());
    m_transition = msg.transition();
    messages::sub_actions::NotificationData data;
    if (!msg.data().UnpackTo(&data))
    {
        throw std::invalid_argument("SubActionImpls::Notification::Deserialize");
    }
    m_category = data.category();
    m_message = data.message();
}

void SubActionImpls::Notification::Parse(
    DBHandler::DatabaseConnection&, const SubActionsRow& result, const UserHeldTransaction&)
{
    m_type = result.actionType;
    google::protobuf::Any any;
    messages::sub_actions::NotificationData data;
    if (result.data.is_null() || !any.ParseFromArray(result.data.blob, result.data.len) || !any.UnpackTo(&data))
    {
        throw std::invalid_argument("SubActionImpls::Notification::Parse(db)");
    }
    m_category = data.category();
    m_message = data.message();
    m_timeout = std::chrono::milliseconds(result.timeout);
    m_transition = result.transition;
}

void SubActionImpls::Notification::InternalExec(
    ActionStorage& actionStorage, WebsocketChannel& sockComm, DeviceRegistry&, UserId, int) const
{
    sockComm.Broadcast(nlohmann::json{{"notification", {{"category", m_category}, {"message", m_message}}}});
}

SubActionImpls::Notification SubActionImpls::Notification::Create(
    uint64_t type, int category, std::string message, duration timeout, bool transition)
{
    Notification a{type};
    a.m_category = category;
    a.m_message = std::move(message);
    a.m_timeout = timeout;
    a.m_transition = transition;
    return a;
}

nlohmann::json SubActionImpls::RecursiveAction::ToJSON() const
{
    return {{"type", m_type}, {"actionId", m_actionId},
        {"timeout", std::chrono::duration_cast<std::chrono::milliseconds>(m_timeout).count()},
        {"transition", m_transition}};
}

messages::SubAction SubActionImpls::RecursiveAction::Serialize() const
{
    messages::SubAction msg;
    msg.set_type(m_type);
    msg.set_timeout(std::chrono::duration_cast<std::chrono::milliseconds>(m_timeout).count());
    msg.set_transition(m_transition);
    messages::sub_actions::RecursiveActionData data;
    data.set_action_id(m_actionId);
    msg.mutable_data()->PackFrom(data);
    return msg;
}

void SubActionImpls::RecursiveAction::Parse(const nlohmann::json& json)
{
    m_type = json.at("type");
    m_actionId = json.at("actionId");
    m_timeout = std::chrono::milliseconds(json.at("timeout"));
    m_transition = json.at("transition");
}

void SubActionImpls::RecursiveAction::Deserialize(const messages::SubAction& msg)
{
    m_type = msg.type();
    m_timeout = std::chrono::milliseconds(msg.timeout());
    m_transition = msg.transition();
    messages::sub_actions::RecursiveActionData data;
    if (!msg.data().UnpackTo(&data))
    {
        throw std::invalid_argument("SubActionImpls::RecursiveAction::Deserialize");
    }
    m_actionId = data.action_id();
}

void SubActionImpls::RecursiveAction::Parse(
    DBHandler::DatabaseConnection&, const SubActionsRow& result, const UserHeldTransaction&)
{
    m_type = result.actionType;
    google::protobuf::Any any;
    messages::sub_actions::RecursiveActionData data;
    if (result.data.is_null() || !any.ParseFromArray(result.data.blob, result.data.len) || !any.UnpackTo(&data))
    {
        throw std::invalid_argument("SubActionImpls::RecursiveAction::Parse(db)");
    }
    m_actionId = data.action_id();
    m_timeout = std::chrono::milliseconds(result.timeout);
    m_transition = result.transition;
}

void SubActionImpls::RecursiveAction::InternalExec(ActionStorage& actionStorage, WebsocketChannel& sockComm,
    DeviceRegistry& deviceReg, UserId user, int recursionDepth) const
{
    Action otherAction = actionStorage.GetAction(m_actionId, user).value();
    otherAction.Execute(actionStorage, sockComm, deviceReg, user, recursionDepth + 1);
}

SubActionImpls::RecursiveAction SubActionImpls::RecursiveAction::Create(
    uint64_t type, uint64_t actionId, duration timeout, bool transition)
{
    RecursiveAction a{type};
    a.m_actionId = actionId;
    a.m_timeout = timeout;
    a.m_transition = transition;
    return a;
}
