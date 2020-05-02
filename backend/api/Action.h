#ifndef _ACTION_H
#define _ACTION_H
#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <json.hpp>

#include "api/action.pb.h"

#include "../api/User.h"
#include "../database/ActionsTable.h"
#include "../database/DBHandler.h"
#include "../utility/FactoryRegistry.h"

class UserHeldTransaction;

class SubActionImpl
{
public:
    using SubActionsRow
        = decltype(GetSelectRow(SubActionsTable(), SubActionsTable().actionType, SubActionsTable().data, SubActionsTable().timeout, SubActionsTable().transition));

public:
    using Ptr = std::shared_ptr<SubActionImpl>;
    using ConstPtr = std::shared_ptr<const SubActionImpl>;
    virtual ~SubActionImpl() = default;
    // Executes SubAction
    virtual void Execute(class ActionStorage& actionStorage, class WebsocketChannel& notificationsChannel,
        class DeviceRegistry& deviceReg, UserId user, int recursionDepth = 0) const = 0;
    virtual nlohmann::json ToJSON() const = 0;
	virtual messages::SubAction Serialize() const = 0;
    // Parse from Json
    virtual void Parse(const nlohmann::json& json) = 0;
	virtual void Deserialize(const messages::SubAction& msg) = 0;
    // Parse from DBResult
    virtual void Parse(DBHandler::DatabaseConnection& dbHandler, const SubActionsRow& result, const UserHeldTransaction&) = 0;
};

/*!
 * \brief A struct containing information about a single SubAction in an Action.
 *
 * Types of actions:
 * \li \c 0: Set an Actor on a node.
 * \li \c 1: Toggle an Actor on a node.
 * \li \c 2: Send a notification.
 * \li \c 3: Set a group of Actor%s (e.g. all lights...) (NYI).
 * \li \c 4: Toggle a group of Actor%s (NYI)
 * \li \c 5: Activate a different action.
 *
 * \see Action
 */
class SubAction
{
public:
    SubAction() = default;
    // Construct from SubActionImpl pointer
    explicit SubAction(SubActionImpl::ConstPtr impl) : m_impl(std::move(impl)) {}
    // Construct from SubActionImpl value
    template <typename T, typename = std::enable_if_t<std::is_base_of<SubActionImpl, std::decay_t<T>>::value>>
    explicit SubAction(T&& impl) : m_impl(std::make_shared<std::decay_t<T>>(std::forward<T>(impl)))
    {}

    // Executes the SubAction
    void Execute(ActionStorage& actionStorage, class WebsocketChannel& notificationsChannel,
        class DeviceRegistry& deviceReg, UserId user, int recursionDepth = 0) const;
    nlohmann::json ToJSON() const;
	messages::SubAction Serialize() const;
    // Just compares pointers for now
    bool operator==(const SubAction& rhs) const { return m_impl->ToJSON() == rhs.m_impl->ToJSON(); }
    // Just compares pointers for now
    bool operator!=(const SubAction& rhs) const { return m_impl->ToJSON() != rhs.m_impl->ToJSON(); }

private:
    SubActionImpl::ConstPtr m_impl;
};

// Contains factory function and config filename for SubActionImpls
struct SubActionInfo
{
    std::function<SubActionImpl::Ptr(uint64_t)> function;
    std::string name;
    SubActionInfo() = default;
    // Construct from nullptr, same as default
    SubActionInfo(std::nullptr_t /*unused*/) {}
    // Construct from factory function and filename
    SubActionInfo(std::function<SubActionImpl::Ptr(uint64_t)> function, std::string name)
        : function(function), name(std::move(name))
    {}
    // Required to determine if not set
    bool operator==(std::nullptr_t /*unused*/) const { return function == nullptr; }
    bool operator!=(std::nullptr_t /*unused*/) const { return !(*this == nullptr); }
};

// Registry for SubActionImpls
class SubActionRegistry : public FactoryRegistry<SubActionInfo>
{
public:
    // Registers standard types (0-5), should only be called once before anything else is registered
    void RegisterDefaultSubActions();
    // Returns implementation of given type
    SubActionImpl::Ptr GetImpl(uint64_t type) const;
    // Parses whole SubAction from json, json["type"] is used for type
    SubAction Parse(const nlohmann::json& value) const;
	SubAction Deserialize(const messages::SubAction& msg) const;
    // Parses whole SubAction from DBResult, column 0 is used for type
    SubAction Parse(DBHandler::DatabaseConnection& dbHandler, const SubActionImpl::SubActionsRow& result, const UserHeldTransaction&) const;
    // Returns all registered types
    const std::vector<SubActionInfo>& GetRegistered() const;
};

/*!
 * \brief A class containing the data of an Action.
 *
 * Each Action represents a set of instructions to execute when a button on the webserver is pressed.
 * It has an \link m_id <code>id</code>\endlink, a \link m_name <code>name</code>\endlink, an \link m_icon
 * <code>icon</code>\endlink, a \link m_color <code>color</code> \endlink, a \link m_actions list \endlink SubAction%s
 * and a \link m_visible visibility flag \endlink.
 */
class Action
{
public:
    /*!
     * \brief Default constructor.
     *
     * Initializes all values to 0 or an empty string.
     */
    Action() : m_id(0), m_name(""), m_icon(""), m_color(0), m_visible(false) {}
    /*!
     * \brief Constructs an Action from the given values.
     * \param id The internal id.
     * \param name The display name.
     * \param icon The icon path to display.
     * \param color Color value of the display color.
     * \param actions List of actions combined in this Action.
     * \param visible User visibility of the Action, default true.
     */
    Action(uint64_t id, std::string name, std::string icon, unsigned int color, std::vector<SubAction> actions,
        bool visible = true)
        : m_id(id),
          m_name(std::move(name)),
          m_icon(std::move(icon)),
          m_color(color),
          m_actions(std::move(actions)),
          m_visible(visible)
    {}

    /*!
     * \brief Getter for the internal id.
     * \returns The internal id.
     */
    uint64_t GetId() const { return m_id; }
    /*!
     * \brief Getter for the display name.
     * \returns The name to display on the website.
     */
    const std::string& GetName() const { return m_name; }
    /*!
     * \brief Getter for the icon path.
     * \returns The path of the icon to display.
     */
    const std::string& GetIcon() const { return m_icon; }
    /*!
     * \brief Getter for the color.
     * \returns The color value to display.
     */
    unsigned int GetColor() const { return m_color; }
    /*!
     * \brief Getter for the list of SubAction%s.
     * \returns The list of SubAction%s executed with this Action.
     */
    const std::vector<SubAction>& GetSubActions() const { return m_actions; }

    /*!
     * \brief Getter for the visibility flag.
     * \returns The visibility for the website.
     */
    bool GetVisibility() const { return m_visible; }

    /*!
     * \brief Setter for the internal id.
     * \param id The new id.
     */
    void SetId(uint64_t id) { m_id = id; }
    /*!
     * \brief Setter for the name.
     * \param name The new name.
     */
    void SetName(const std::string& name) { m_name = name; }
    /*!
     * \brief Setter for the icon path.
     * \param icon The new icon path.
     */
    void SetIcon(const std::string& icon) { m_icon = icon; }
    /*!
     * \brief Setter for the color.
     * \param color The new color value.
     */
    void SetColor(unsigned int color) { m_color = color; }
    /*!
     * \brief Setter for the action list.
     * \param actions The new list of actions.
     */
    void SetActions(std::vector<SubAction> actions) { m_actions = std::move(actions); }

    /*!
     * \brief Setter for the visibility.
     * \param visible The new visibility on the website.
     */
    void SetVisible(bool visible) { m_visible = visible; }

    /*!
     * \brief Executes this Action's SubAction%s.
     * \param dbHandler The DBHandler to request other Action%s and execute them.
     * \param notificationsChannel The WebsocketChannel to send notifications.
     * \param deviceReg The DeviceRegistry to access device%s.
     * \param recursionDepth The number of sub-action calls this Execute() is deep. Should not be used manually.
     */
    void Execute(ActionStorage& actionStorage, class WebsocketChannel& notificationsChannel,
        class DeviceRegistry& deviceReg, UserId user, int recursionDepth = 0) const;

    /*!
     * \brief Converts this Action to a JSON object for sending.
     * \returns The converted JSON object.
     */
    nlohmann::json ToJson() const;
	messages::Action Serialize() const;

    /*!
     * \brief Parses an Action and all its SubAction%s from a JSON value.
     * \param value The JSON value. Has to be a JSON object with all fields.
     * \returns The parsed Action.
     * \throws Json::Exception if a JSON parse error occurs.
     */
    static Action Parse(const nlohmann::json& value, const SubActionRegistry& registry);
	static Action Deserialize(const messages::Action& msg, const SubActionRegistry& registry);

	friend bool operator==(const Action& left, const Action& right);
	friend bool operator!=(const Action& left, const Action& right);

public:
    /*!
     * \brief The maximum number of stacked sub-calls.
     */
    constexpr static int s_maxRecursion = 10;

private:
    /*!
     * \brief The internal ID of the Action.
     *
     * This id is only used for identification in the database and on the website. It stays constant for an Action.
     *
     * This int is 64 bit because SQL rowids have that size.
     */
    uint64_t m_id;
    /*!
     * \brief The name of the Action displayed on the website.
     */
    std::string m_name;
    /*!
     * \brief The icon name of the Action.
     *
     * This string contains the path of the image file.
     */
    std::string m_icon;
    /*!
     * \brief The color value of the Action.
     *
     * The color is used to display different styles of Actions on the website.
     */
    unsigned int m_color;
    /*!
     * \brief The list of actions this Action executes.
     *
     * These actions might be instantaneous, or have a delay depending on their settings.
     */
    std::vector<SubAction> m_actions;
    /*!
     * \brief Whether this Action should be displayed on the website.
     *
     * Set to false for internal Action%s, e.g. in combination with Rule%s.
     */
    bool m_visible;
};

#endif
