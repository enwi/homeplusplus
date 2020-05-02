#ifndef _DEVICE_API_H
#define _DEVICE_API_H

#include <json.hpp>

#include "Action.h"
#include "DeviceType.h"
#include "Rule.h"

#include "../events/EventSystem.h"
#include "../utility/SimpleRange.h"

class IDeviceAPI
{
public:
    // Should not be used for work, use Shutdown() instead
    virtual ~IDeviceAPI() = default;

    // Used to initialize and read config, called before RegisterXXX() and Start()
    virtual void Initialize(nlohmann::json& config) = 0;

    // Used to start threads, called after Initialize() and RegisterXXX() of all loaded APIs
    virtual void Start() {}

    // Used to stop threads, called last (only if Start() was successful)
    virtual void Shutdown() {}

    // Called after Initialize(), RegisterRuleConditions() and RegisterSubActinons() and before Start()
    virtual void RegisterEventHandlers(EventSystem& /*evSys*/) {}

    // Called after Initialize() and before Start()
    virtual void RegisterRuleConditions(RuleConditions::Registry& /*registry*/) {}

    // Called after Initialize() and before Start()
    virtual void RegisterSubActions(SubActionRegistry& /*registry*/) {}

    // Register different device types (not used currently)
    virtual void RegisterDeviceTypes(DeviceTypeRegistry& /*registry*/) {}
    // Add/Remove/Change devices at beginning when external changes happened
    // storage only contains devices of this plugin
    virtual void SynchronizeDevices(DeviceStorage& /*storage*/) {}

    virtual const char* GetAPIId() const noexcept = 0;
};

//-Register own event types
//-Register own event handlers
//-Register own rule conditions
//-Register own actions
// Register configuration page
// Register device types
// Handle device configuration
// Way to display devices/-actions/-conditions/configuration
// Way to save devices
// Devices:
//  Display in node list
//  Own/Customized? nodeView page
//  Sensors + Actors

#endif