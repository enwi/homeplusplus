#ifndef _HUE_API_H
#define _HUE_API_H

#include <Hue.h>
#include <HueLight.h>
#ifdef _MSC_VER
#include <WinHttpHandler.h>
#else
#include <LinHttpHandler.h>
#endif

#include "HueDeviceType.h"

#include "../../api/DeviceAPI.h"
#include "../../api/DeviceStorage.h"
// #include "../../events/EventSystem.h"

class HueAPI : public IDeviceAPI
{
public:
    HueAPI(UserId apiUser);

    void Initialize(nlohmann::json& config) override;

    void RegisterEventHandlers(EventSystem& evSys) override;

    void RegisterRuleConditions(RuleConditions::Registry& registry) override;

    void RegisterSubActions(SubActionRegistry& registry) override;

    const char* GetAPIId() const noexcept override { return "HUEAPI_0.0"; }

    void RegisterDeviceTypes(DeviceTypeRegistry& registry) override;

    void SynchronizeDevices(DeviceStorage& storage) override;

    Hue& GetHue() { return m_hue; }

    static HueAPI& Get() { return *s_instance; }

private:
    Device CreateDeviceFromLight(const HueLight& light) const;
    void UpdateDeviceFromLight(Device& device, DeviceStorage& storage, const HueLight& light) const;

private:
    static HueAPI* s_instance;
    UserId m_apiUser;
    uint64_t m_setLightAction = 0;
    std::shared_ptr<IHttpHandler> handler; // needs to be before Hue m_hue!!!
    Hue m_hue;
    HueDeviceType* m_hueLightType;
};

#endif
