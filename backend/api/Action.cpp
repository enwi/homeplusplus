#include "Action.h"

#include <algorithm>
#include <cctype>
#include <thread>

#include "DeviceRegistry.h"
#include "SubActionImpls.h"

#include "../communication/WebsocketChannel.h"
#include "../database/DBHandler.h"

void Action::Execute(ActionStorage& actionStorage, WebsocketChannel& notificationsChannel, DeviceRegistry& deviceReg,
    UserId user, int recursionDepth) const
{
    if (recursionDepth > s_maxRecursion && m_actions.size() > 0)
    {
        Res::Logger().Warning("Maximum recursion reached in Action::Execute()");
        return;
    }
    for (const SubAction& action : m_actions)
    {
        action.Execute(actionStorage, notificationsChannel, deviceReg, user, recursionDepth);
    }
}

nlohmann::json Action::ToJson() const
{
    nlohmann::json result {{"id", m_id}, {"name", m_name}, {"icon", m_icon}, {"color", m_color}, {"visible", m_visible},
        {"subActions", nlohmann::json::array()}};
    for (auto it = m_actions.begin(); it != m_actions.end(); ++it)
    {
        result["subActions"].emplace_back(it->ToJSON());
    }
    return result;
}

messages::Action Action::Serialize() const
{
    messages::Action msg;
    msg.set_id(m_id);
    msg.set_name(m_name);
    msg.set_icon(m_icon);
    msg.set_color(m_color);
    msg.set_visible(m_visible);
    for (const SubAction& subAction : m_actions)
    {
        msg.mutable_sub_actions()->Add(subAction.Serialize());
    }
    return msg;
}

Action Action::Parse(const nlohmann::json& value, const SubActionRegistry& registry)
{
    int id = 0;
    if (value.count("id"))
    {
        id = value.at("id");
    }
    std::string name = value.at("name");
    std::string icon = value.at("icon");
    unsigned int color = value.at("color");
    nlohmann::json actionsJson = value.at("subActions");
    bool visible = true;
    if (value.count("visible"))
    {
        visible = value.at("visible");
    }
    std::vector<SubAction> subActions;
    for (const nlohmann::json& v : actionsJson)
    {
        subActions.push_back(registry.Parse(v));
    }
    return Action(id, name, icon, color, std::move(subActions), visible);
}

Action Action::Deserialize(const messages::Action& msg, const SubActionRegistry& registry)
{
    std::vector<SubAction> subActions;
    for (const messages::SubAction& v : msg.sub_actions())
    {
        subActions.push_back(registry.Deserialize(v));
    }
    return Action(msg.id(), msg.name(), msg.icon(), msg.color(), std::move(subActions), msg.visible());
}

void SubAction::Execute(ActionStorage& actionStorage, WebsocketChannel& notificationsChannel, DeviceRegistry& deviceReg,
    UserId user, int recursionDepth) const
{
    assert(m_impl != nullptr);
    m_impl->Execute(actionStorage, notificationsChannel, deviceReg, user, recursionDepth);
}

nlohmann::json SubAction::ToJSON() const
{
    assert(m_impl != nullptr);
    return m_impl->ToJSON();
}

messages::SubAction SubAction::Serialize() const
{
    assert(m_impl != nullptr);
    return m_impl->Serialize();
}

void SubActionRegistry::RegisterDefaultSubActions()
{
    if (!m_factories.empty())
    {
        Res::Logger().Warning("SubActionRegistry::RegisterDefaultSubActions was called after sub "
                              "actions were registered!");
    }
    else
    {
        using namespace SubActionImpls;
        Register(SubActionInfo {[](uint64_t id) { return std::make_shared<DeviceSet>(id); }, "deviceSet"}, 0);
        Register(SubActionInfo {[](uint64_t id) { return std::make_shared<DeviceToggle>(id); }, "deviceToggle"}, 1);
        Register(SubActionInfo {[](uint64_t id) { return std::make_shared<Notification>(id); }, "notification"}, 2);
        // Not yet implemented
        // Register([](uint64_t id) { return std::make_shared<GroupActorSubActionImpl>(id); },3);
        // Register([](uint64_t id) { return std::make_shared<GroupToggleSubActionImpl>(id); }, 4);
        Register(
            SubActionInfo {[](uint64_t id) { return std::make_shared<RecursiveAction>(id); }, "recursiveAction"}, 5);
    }
}

SubActionImpl::Ptr SubActionRegistry::GetImpl(uint64_t type) const
{
    if (m_factories.size() > type && m_factories[type] != nullptr)
    {
        return m_factories[type].function(type);
    }
    else
    {
        Res::Logger().Warning("Unknown sub action implementation type: " + std::to_string(type));
        throw std::out_of_range(
            "SubActionRegistry::GetImpl: Unknown sub action implementation type: " + std::to_string(type));
    }
}

SubAction SubActionRegistry::Parse(const nlohmann::json& value) const
{
    uint64_t type = value.at("type");
    SubActionImpl::Ptr instance = GetImpl(type);
    assert(instance != nullptr);
    instance->Parse(value);
    return SubAction(instance);
}

SubAction SubActionRegistry::Deserialize(const messages::SubAction& msg) const
{
    SubActionImpl::Ptr instance = GetImpl(msg.type());
    assert(instance != nullptr);
    instance->Deserialize(msg);
    return SubAction(instance);
}

SubAction SubActionRegistry::Parse(DBHandler::DatabaseConnection& dbHandler, const SubActionImpl::SubActionsRow& result,
    const UserHeldTransaction& transaction) const
{
    uint64_t type = result.actionType;
    SubActionImpl::Ptr instance = GetImpl(type);
    assert(instance != nullptr);
    instance->Parse(dbHandler, result, transaction);
    return SubAction(instance);
}

const std::vector<SubActionInfo>& SubActionRegistry::GetRegistered() const
{
    return m_factories;
}

bool operator==(const Action& left, const Action& right)
{
    return left.m_id == right.m_id && left.m_name == right.m_name && left.m_icon == right.m_icon
        && left.m_color == right.m_color && left.m_actions == right.m_actions && left.m_visible == right.m_visible;
}

bool operator!=(const Action& left, const Action& right)
{
    return !(left == right);
}