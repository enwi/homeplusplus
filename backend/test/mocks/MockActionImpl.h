#pragma once

#include <absl/types/any.h>
#include <gmock/gmock.h>

#include "JsonWrapper.h"

#include "api/Action.h"
#include "database/DBHandler.h"

class MockActionImpl : public SubActionImpl
{
public:
    MockActionImpl() = default;
    MockActionImpl(std::size_t type) : type(type) {}

    void UseDefaultJson()
    {
        using namespace ::testing;
        ON_CALL(*this, ToJSONImpl()).WillByDefault(Invoke([&]() {
            return JsonWrapper(nlohmann::json{{"type", type}, {"data", "some test data"}});
        }));
        ON_CALL(*this, ParseImpl(_)).WillByDefault(Invoke([&](const JsonWrapper& json) {
            nlohmann::json jsonCasted = json.get();
            type = jsonCasted.at("type");
            std::string data = jsonCasted.at("data");
            EXPECT_EQ("some test data", data);
        }));
    }

    MOCK_CONST_METHOD5(Execute,
        void(class ActionStorage& actionStorage, class WebsocketChannel& notificationsChannel,
            class DeviceRegistry& deviceReg, UserId user, int recursionDepth));
    nlohmann::json ToJSON() const override { return ToJSONImpl().get(); }
	MOCK_CONST_METHOD0(Serialize, messages::SubAction());
    void Parse(const nlohmann::json& json) override { ParseImpl(JsonWrapper(json)); }
	MOCK_METHOD1(Deserialize, void(const messages::SubAction&));
    // Cannot use nlohmann::json, so contained in any
    MOCK_METHOD1(ParseImpl, void(const JsonWrapper& json));
    MOCK_CONST_METHOD0(ToJSONImpl, JsonWrapper());
    MOCK_CONST_METHOD0(ToDBValues, std::array<class DBValue, 6>());
    MOCK_METHOD3(
        Parse, void(DBHandler::DatabaseConnection& dbHandler, const SubActionsRow& result, const UserHeldTransaction&));

public:
    std::size_t type = 0;
};
