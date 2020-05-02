#pragma once

#include <Hue.h>

#include "../../api/DeviceType.h"

class HueDeviceType : public DeviceType
{
public:
    HueDeviceType(Hue& hue, UserId apiUser);

    absl::string_view GetName() const override { return "hueLight"; }
    const Metadata& GetDeviceMetadata() const override { return m_meta; }
    bool ValidateUpdate(absl::string_view property, const nlohmann::json& value, UserId user) const override;
    void OnUpdate(absl::string_view property, Device& device, UserId user) const override;

private:
    Metadata m_meta;
    UserId m_apiUser;
    Hue* m_hue;
};