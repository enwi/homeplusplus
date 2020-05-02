#include "api/DeviceRegistry.h"

#include <cstdio>
#include <memory>
#include <stdexcept>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
//#include <stdlib.h>
#include <typeinfo>

#include "../mocks/MockDevice.h"
#include "../mocks/MockDeviceAPI.h"
#include "api/Action.h"
#include "api/Rule.h"
#include "events/EventSystem.h"

const std::string test_api_id = "Halla12324";
const std::string test_api_folder = "testconfig";
const std::string test_api_file_path = test_api_folder + "/" + test_api_id + "_config.json";
TEST(DeviceRegistry, RegisterDeviceAPI)
{
    using namespace ::testing;
    std::unique_ptr<MockDeviceAPI> mockdapi1 = std::make_unique<MockDeviceAPI>();
    EXPECT_CALL(*mockdapi1, GetAPIIdMock()).Times(1).WillOnce(Return(test_api_id.c_str()));

    DeviceRegistry test_dreg;
    test_dreg.RegisterDeviceAPI(std::move(mockdapi1));

    EXPECT_NO_THROW((void)dynamic_cast<const MockDeviceAPI&>(test_dreg.GetAPI(test_api_id)));
}

TEST(DeviceRegistry, RemoveDeviceAPI)
{
    using namespace ::testing;
    std::unique_ptr<MockDeviceAPI> mockdapi1 = std::make_unique<MockDeviceAPI>();
    EXPECT_CALL(*mockdapi1, GetAPIIdMock()).Times(1).WillOnce(Return(test_api_id.c_str()));

    DeviceRegistry test_dreg;
    test_dreg.RegisterDeviceAPI(std::move(mockdapi1));
    test_dreg.RemoveDeviceAPI(test_api_id);

    EXPECT_THROW((void)dynamic_cast<const MockDeviceAPI&>(test_dreg.GetAPI(test_api_id)), std::out_of_range);
}

TEST(DeviceRegistry, GetAllAPIs)
{
    using namespace ::testing;
    std::unique_ptr<MockDeviceAPI> mockdapi1 = std::make_unique<MockDeviceAPI>();
    MockDeviceAPI* mockdapip1 = mockdapi1.get();
    std::unique_ptr<MockDeviceAPI> mockdapi2 = std::make_unique<MockDeviceAPI>();
    MockDeviceAPI* mockdapip2 = mockdapi2.get();
    /*EXPECT_CALL(*mockdapi1, GetAPIIdMock())
        .Times(1)
        .WillOnce(Return(test_api_id.c_str()));*/

    DeviceRegistry test_dreg;
    test_dreg.RegisterDeviceAPI(std::move(mockdapi1));
    test_dreg.RegisterDeviceAPI(std::move(mockdapi2));

    EXPECT_THAT(test_dreg.GetAllAPIs(),
        ElementsAre(Truly([&](const std::unique_ptr<IDeviceAPI>& p) { return p.get() == mockdapip1; }),
            Truly([&](const std::unique_ptr<IDeviceAPI>& p) { return p.get() == mockdapip2; })));
    const DeviceRegistry& test_const_dreg = test_dreg;
    EXPECT_THAT(test_const_dreg.GetAllAPIs(),
        ElementsAre(Truly([&](const std::unique_ptr<IDeviceAPI>& p) { return p.get() == mockdapip1; }),
            Truly([&](const std::unique_ptr<IDeviceAPI>& p) { return p.get() == mockdapip2; })));
}

TEST(DeviceRegistry, GetAllDevices)
{
    using namespace ::testing;
    std::unique_ptr<MockDeviceAPI> mockdapi1 = std::make_unique<MockDeviceAPI>();
    std::unique_ptr<MockDevice> mockd1 = std::make_unique<MockDevice>();
    const MockDevice* mockdp1 = mockd1.get();
    std::vector<std::unique_ptr<MockDevice>> mockd_vector;
    mockd_vector.emplace_back(std::move(mockd1));
    ::Range<IDevice> idevrange = MakeOwningRange<IDevice, DoubleDereferencer<IDevice>>(std::move(mockd_vector));
    EXPECT_CALL(*mockdapi1, GetDevices())
        .Times(2)
        .WillOnce(Return(MakeOwningRange<IDevice>(std::vector<MockDevice>{})))
        .WillOnce(Return(idevrange));
    DeviceRegistry test_dreg;
    test_dreg.RegisterDeviceAPI(std::move(mockdapi1));

    EXPECT_THAT(test_dreg.GetAllDevices(), ElementsAre());
    EXPECT_THAT(test_dreg.GetAllDevices(),
        ElementsAre(Truly([&](const IDevice& p) { return static_cast<const MockDevice*>(&p) == mockdp1; })));
}

TEST(DeviceRegistry, FindDevice)
{
    using namespace ::testing;
    std::unique_ptr<MockDeviceAPI> mockdapi1 = std::make_unique<MockDeviceAPI>();
    std::unique_ptr<MockDevice> mockd1 = std::make_unique<MockDevice>();
    const MockDevice* mockdp1 = mockd1.get();
    /*EXPECT_CALL(*mockd1, GetIdMock())
        .Times(1)
        .WillOnce(Return(125));*/
    EXPECT_CALL(*mockdapi1, GetDevice(125))
        .Times(2)
        .WillOnce(Return(ByMove(nullptr)))
        .WillOnce(Return(ByMove(std::move(mockd1))));
    DeviceRegistry test_dreg;
    test_dreg.RegisterDeviceAPI(std::move(mockdapi1));

    EXPECT_EQ(test_dreg.FindDevice(125).get(), nullptr);
    EXPECT_EQ(test_dreg.FindDevice(125).get(), mockdp1);
}

TEST(DeviceRegistry, GetAPIDevices)
{
    using namespace ::testing;
    std::unique_ptr<MockDeviceAPI> mockdapi1 = std::make_unique<MockDeviceAPI>();
    std::unique_ptr<MockDevice> mockd1 = std::make_unique<MockDevice>();
    const MockDevice* mockdp1 = mockd1.get();
    std::vector<std::unique_ptr<MockDevice>> mockd_vector;
    mockd_vector.emplace_back(std::move(mockd1));
    ::Range<IDevice> idevrange = MakeOwningRange<IDevice, DoubleDereferencer<IDevice>>(std::move(mockd_vector));
    EXPECT_CALL(*mockdapi1, GetDevices())
        .Times(2)
        .WillOnce(Return(MakeOwningRange<IDevice>(std::vector<MockDevice>{})))
        .WillOnce(Return(idevrange));
    EXPECT_CALL(*mockdapi1, GetAPIIdMock()).Times(2).WillRepeatedly(Return(test_api_id.c_str()));
    DeviceRegistry test_dreg;
    test_dreg.RegisterDeviceAPI(std::move(mockdapi1));

    EXPECT_THAT(test_dreg.GetAPIDevices(test_api_id), ElementsAre());
    EXPECT_THAT(test_dreg.GetAPIDevices(test_api_id),
        ElementsAre(Truly([&](const IDevice& p) { return static_cast<const MockDevice*>(&p) == mockdp1; })));
}

TEST(DeviceRegistry, GetAPI)
{
    using namespace ::testing;
    std::unique_ptr<MockDeviceAPI> mockdapi1 = std::make_unique<MockDeviceAPI>();
    EXPECT_CALL(*mockdapi1, GetAPIIdMock())
        .Times(4)
        .WillOnce(Return(""))
        .WillOnce(Return(test_api_id.c_str()))
        .WillOnce(Return(""))
        .WillOnce(Return(test_api_id.c_str()));

    DeviceRegistry test_dreg;
    test_dreg.RegisterDeviceAPI(std::move(mockdapi1));

    EXPECT_THROW((void)dynamic_cast<const MockDeviceAPI&>(test_dreg.GetAPI(test_api_id)), std::out_of_range);
    EXPECT_NO_THROW((void)dynamic_cast<const MockDeviceAPI&>(test_dreg.GetAPI(test_api_id)));

    const DeviceRegistry& test_const_dreg = test_dreg;
    EXPECT_THROW((void)dynamic_cast<const MockDeviceAPI&>(test_const_dreg.GetAPI(test_api_id)), std::out_of_range);
    EXPECT_NO_THROW((void)dynamic_cast<const MockDeviceAPI&>(test_const_dreg.GetAPI(test_api_id)));
}

TEST(DeviceRegistry, InitAPIs)
{
    using namespace ::testing;
    const std::string command = "mkdir " + test_api_folder;
    system(command.c_str());
    nlohmann::json value = nlohmann::json::object();
    std::ofstream stream(test_api_file_path);
    stream << std::setw(4) << value;
    stream.close();
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    std::unique_ptr<MockDeviceAPI> mockdapi1 = std::make_unique<MockDeviceAPI>();
    EXPECT_CALL(*mockdapi1, GetAPIIdMock()).Times(AtLeast(1)).WillRepeatedly(Return(test_api_id.c_str()));
    EXPECT_CALL(*mockdapi1, Initialize(value))
        .Times(AtLeast(1))
        .WillOnce(Return())
        .WillOnce(Throw(std::runtime_error("Test runtime error")))
        .WillRepeatedly(Return());
    EXPECT_CALL(*mockdapi1, RegisterEventHandlers(_)).Times(AtLeast(1)).WillRepeatedly(Return());
    EXPECT_CALL(*mockdapi1, RegisterRuleConditions(_)).Times(AtLeast(1)).WillRepeatedly(Return());
    EXPECT_CALL(*mockdapi1, RegisterSubActions(_)).Times(AtLeast(1)).WillRepeatedly(Return());

    DeviceRegistry test_dreg;
    test_dreg.RegisterDeviceAPI(std::move(mockdapi1));

    EventSystem evSys;
    RuleConditions::Registry rcReg;
    SubActionRegistry suaReg;
    test_dreg.InitAPIs(test_api_folder, evSys, rcReg, suaReg);
    test_dreg.InitAPIs(test_api_folder, evSys, rcReg, suaReg);
    std::remove(test_api_file_path.c_str());
}

TEST(DeviceRegistry, Start)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    std::unique_ptr<MockDeviceAPI> mockdapi1 = std::make_unique<MockDeviceAPI>();
    EXPECT_CALL(*mockdapi1, Start())
        .Times(2)
        .WillOnce(Return())
        .WillOnce(Throw(std::runtime_error("Test runtime error")));
    EXPECT_CALL(*mockdapi1, GetAPIIdMock()).Times(1).WillOnce(Return(test_api_id.c_str()));

    DeviceRegistry test_dreg;
    test_dreg.RegisterDeviceAPI(std::move(mockdapi1));

    test_dreg.Start();
    test_dreg.Start();
}

TEST(DeviceRegistry, Shutdown)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    std::unique_ptr<MockDeviceAPI> mockdapi1 = std::make_unique<MockDeviceAPI>();
    EXPECT_CALL(*mockdapi1, Shutdown())
        .Times(2)
        .WillOnce(Return())
        .WillOnce(Throw(std::runtime_error("Test runtime error")));
    EXPECT_CALL(*mockdapi1, GetAPIIdMock()).Times(1).WillOnce(Return(test_api_id.c_str()));

    DeviceRegistry test_dreg;
    test_dreg.RegisterDeviceAPI(std::move(mockdapi1));

    test_dreg.Shutdown();
    test_dreg.Shutdown();
}
