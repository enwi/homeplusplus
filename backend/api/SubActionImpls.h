#ifndef _SUB_ACTION_IMPLS_H
#define _SUB_ACTION_IMPLS_H

#include <absl/strings/string_view.h>

#include "Action.h"
#include "Device.h"

#include "../communication/WebsocketChannel.h"

namespace SubActionImpls
{
    // T = child class which inherits from BaseImpl
    template <typename T>
    class BaseImpl : public SubActionImpl
    {
    public:
        using duration = std::chrono::steady_clock::duration;
        BaseImpl(uint64_t typeId = 0) : m_type(typeId) {}

        virtual void Execute(ActionStorage& actionStorage, WebsocketChannel& notificationsChannel,
            DeviceRegistry& deviceReg, UserId user, int recursionDepth = 0) const override;
        virtual void InternalExec(ActionStorage& actionStorage, WebsocketChannel& notificationsChannel,
            DeviceRegistry& deviceReg, UserId user, int recursionDepth) const = 0;

    protected:
        uint64_t m_type;
        duration m_timeout = std::chrono::milliseconds(0);
        bool m_transition = false;
    };

    // SubAction setting an Actor to a value
    class DeviceSet : public BaseImpl<DeviceSet>
    {
    public:
        DeviceSet(uint64_t typeId = 0) : BaseImpl(typeId), m_deviceId(0) {}

        virtual nlohmann::json ToJSON() const override;
		virtual messages::SubAction Serialize() const override;
        virtual void Parse(const nlohmann::json& json) override;
		virtual void Deserialize(const messages::SubAction& msg) override;
        virtual void Parse(DBHandler::DatabaseConnection& dbHandler, const SubActionsRow& result, const UserHeldTransaction&) override;
        virtual void InternalExec(ActionStorage& actionStorage, WebsocketChannel& notificationsChannel,
            DeviceRegistry& deviceReg, UserId user, int recursionDepth = 0) const override;

        static DeviceSet Create(uint64_t type, DeviceId deviceId, absl::string_view property, const std::string& value,
            duration timeout, bool transition);

    private:
        DeviceId m_deviceId;
        std::string m_property;
        nlohmann::json m_value;
    };

    // SubAction toggling an Actor
    class DeviceToggle : public BaseImpl<DeviceToggle>
    {
    public:
        DeviceToggle(uint64_t typeId = 0) : BaseImpl(typeId), m_deviceId(0) {}

        virtual nlohmann::json ToJSON() const override;
		virtual messages::SubAction Serialize() const override;
        virtual void Parse(const nlohmann::json& json) override;
		virtual void Deserialize(const messages::SubAction& msg) override;
		virtual void Parse(DBHandler::DatabaseConnection& dbHandler, const SubActionsRow& result, const UserHeldTransaction&) override;
        virtual void InternalExec(ActionStorage& actionStorage, WebsocketChannel& notificationsChannel,
            DeviceRegistry& deviceReg, UserId user, int recursionDepth = 0) const override;

        static DeviceToggle Create(
            uint64_t type, DeviceId deviceId, absl::string_view property, duration timeout, bool transition);

    private:
        DeviceId m_deviceId;
        std::string m_property;
    };

    // SubAction displaying a notification
    class Notification : public BaseImpl<Notification>
    {
    public:
        Notification(uint64_t typeId = 0) : BaseImpl(typeId), m_category(0) {}

        virtual nlohmann::json ToJSON() const override;
		virtual messages::SubAction Serialize() const override;
        virtual void Parse(const nlohmann::json& json) override;
		virtual void Deserialize(const messages::SubAction& msg) override;
		virtual void Parse(DBHandler::DatabaseConnection& dbHandler, const SubActionsRow& result, const UserHeldTransaction&) override;
        virtual void InternalExec(ActionStorage& actionStorage, WebsocketChannel& notificationsChannel,
            DeviceRegistry& deviceReg, UserId user, int recursionDepth = 0) const override;

        static Notification Create(uint64_t type, int category, std::string message, duration timeout, bool transition);

    private:
        int m_category;
        std::string m_message;
    };

    // SubAction calling another Action
    class RecursiveAction : public BaseImpl<RecursiveAction>
    {
    public:
        RecursiveAction(uint64_t typeId = 0) : BaseImpl(typeId), m_actionId(0) {}

        virtual nlohmann::json ToJSON() const override;
		virtual messages::SubAction Serialize() const override;
        virtual void Parse(const nlohmann::json& json) override;
		virtual void Deserialize(const messages::SubAction& msg) override;
		virtual void Parse(DBHandler::DatabaseConnection& dbHandler, const SubActionsRow& result, const UserHeldTransaction&) override;
        virtual void InternalExec(ActionStorage& actionStorage, WebsocketChannel& notificationsChannel,
            DeviceRegistry& deviceReg, UserId user, int recursionDepth = 0) const override;

        static RecursiveAction Create(uint64_t type, uint64_t actionId, duration timeout, bool transition);

    private:
        uint64_t m_actionId;
    };
} // namespace SubActionImpls

// Implementation of SubActionImpls::BaseImpl::Execute, requires additional includes
#include "DeviceRegistry.h"

#include "../api/Resources.h"
#include "../communication/WebsocketCommunication.h"
#include "../database/DBHandler.h"
#include "../utility/Logger.h"

template <typename T>
void SubActionImpls::BaseImpl<T>::Execute(ActionStorage& actionStorage, WebsocketChannel& notificationsChannel,
    DeviceRegistry& deviceReg, UserId user, int recursionDepth) const
{
    static_assert(std::is_base_of<BaseImpl, T>::value, "BaseSubActionImpl must be inherited with T = Child class");
    if (m_timeout == std::chrono::milliseconds(0))
    {
        InternalExec(actionStorage, notificationsChannel, deviceReg, user, recursionDepth);
    }
    else
    {
        if (m_timeout > std::chrono::minutes(1))
        {
            Res::Logger().Warning("Timeout in SubAction is longer than one minute, shortened to 1 min: "
                + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(m_timeout).count()) + "s");
        }
        const auto timeout = std::min<std::chrono::steady_clock::duration>(m_timeout, std::chrono::minutes(1));
        // copy this, timeout and recursionDepth, because they might already be deleted when the thread runs
        std::thread([&, hack = *static_cast<const T*>(this), timeout, recursionDepth]() {
            const BaseImpl& self = static_cast<const BaseImpl&>(hack);
            std::this_thread::sleep_for(timeout);
            self.InternalExec(actionStorage, notificationsChannel, deviceReg, user, recursionDepth);
        })
            .detach();
    }
}

#endif
