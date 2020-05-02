#include <gtest/gtest.h>
#include <sqlpp11/insert.h>

#include "../communication/TestWebsocketCommunication.h"
#include "database/DBHandler.h"
#include "database/UsersTable.h"
#include "events/AuthEventHandler.h"

class AuthEventHandlerTest : public ::testing::Test
{
public:
    AuthEventHandlerTest() : dbHandler(":memory:"), handler(dbHandler, sockComm.ws, authenticator) { InitUsersTable(); }

    void InitUsersTable()
    {
        const char* usersTable = "CREATE TABLE users(user_id INTEGER PRIMARY KEY NOT NULL, user_name VARCHAR NOT NULL, "
                                 "user_pwhash VARCHAR, user_priority INTEGER NOT NULL DEFAULT 0, last_login TIMESTAMP, "
                                 "login_attempts INTEGER DEFAULT 0, picture BLOB DEFAULT NULL);";
        dbHandler.GetDatabase().execute(usersTable);
    }

    Authenticator authenticator;
    DBHandler dbHandler;
    TestWebsocketCommunication sockComm;
    AuthEventHandler handler;
};

TEST_F(AuthEventHandlerTest, CallOperator)
{
    EXPECT_EQ(
        PostEventState::notHandled, handler(Events::SocketConnectEvent(websocketpp::connection_hdl(), absl::nullopt)));
    EXPECT_EQ(PostEventState::notHandled,
        handler(Events::SocketDisconnectEvent(websocketpp::connection_hdl(), absl::nullopt)));
    EXPECT_EQ(PostEventState::notHandled,
        handler(
            Events::SocketMessageEvent(websocketpp::connection_hdl(), nullptr, {{"no_login", "t"}}, absl::nullopt)));
}

TEST_F(AuthEventHandlerTest, ValidateLogin)
{
    using namespace ::testing;
    // User not found
    {
        EXPECT_EQ("", handler.ValidateLogin("user", "asgetse"));
        EXPECT_EQ("", handler.ValidateLogin("", "asgetse"));
        EXPECT_EQ("", handler.ValidateLogin("user", "1616"));
    }
    UsersTable users;
    auto& db = dbHandler.GetDatabase();

    const std::string userName = "user13";
    const std::string password = "awg89j2oj6kwlejrasdf";
    const std::string hash = authenticator.CreatePasswordHash(password);
    const uint64_t userId = 0x236641;

    db(insert_into(users).set(users.userId = userId, users.userPwhash = hash, users.userName = userName));
    // Invalid password
    {
        EXPECT_EQ("", handler.ValidateLogin(userName, "asgjeio"));
        EXPECT_EQ("", handler.ValidateLogin(userName, ""));
        EXPECT_EQ("", handler.ValidateLogin(userName, "agiuojg"));
    }
    // Valid password
    {
        const std::string token = handler.ValidateLogin(userName, password);
        EXPECT_FALSE(token.empty());
        nlohmann::json json = authenticator.ValidateJWTToken(token);
        EXPECT_EQ("sh-auth-event-handler", json.at("iss"));
        EXPECT_EQ(std::to_string(userId), json.at("sub"));
        using namespace std::chrono;
        const int64_t expires
            = duration_cast<seconds>(system_clock::now().time_since_epoch() + authenticator.s_tokenExpiration).count();
        EXPECT_LE(json.at("exp"), expires);
    }
}

TEST_F(AuthEventHandlerTest, HandleLogin)
{
    using namespace ::testing;
    auto connection = std::make_shared<MockWebsocketConnection>();
    websocketpp::connection_hdl hdl = connection;
    // Invalid login command
    {
        const Events::SocketMessageEvent event1{hdl, nullptr, nlohmann::json{{"login", "test"}}, absl::nullopt};
        EXPECT_EQ(PostEventState::notHandled, handler(event1));
        const Events::SocketMessageEvent event2{
            hdl, nullptr, nlohmann::json{{"login", {{"username", "ab"}, {"missing_password", ""}}}}, absl::nullopt};
        EXPECT_EQ(PostEventState::notHandled, handler(event2));
        const Events::SocketMessageEvent event3{
            hdl, nullptr, nlohmann::json{{"login", {{"missing_username", "ab"}, {"password", ""}}}}, absl::nullopt};
        EXPECT_EQ(PostEventState::notHandled, handler(event3));
    }
    const nlohmann::json failedResponse{{"authFailed", true}};
    // User not found
    {
        const nlohmann::json payload{{"login", {{"username", "uasdr2"}, {"password", "asd879435"}}}};
        const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
        EXPECT_CALL(sockComm.GetServer(),
            send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }), failedResponse.dump(),
                websocketpp::frame::opcode::text));
        EXPECT_EQ(PostEventState::handled | PostEventState::consumeEvent, handler(event));
    }
    const std::string userName = "abcjtuiwe";
    const std::string password = "astga9u68923";

    const uint64_t userId = 0x4589312;
    UsersTable users;
    auto& db = dbHandler.GetDatabase();
    db(insert_into(users).set(users.userId = userId, users.userPwhash = authenticator.CreatePasswordHash(password),
        users.userName = userName));

    // Invalid password
    {
        const nlohmann::json payload{{"login", {{"username", userName}, {"password", "atge623"}}}};
        const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
        EXPECT_CALL(sockComm.GetServer(),
            send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }), failedResponse.dump(),
                websocketpp::frame::opcode::text));
        EXPECT_EQ(PostEventState::handled | PostEventState::consumeEvent, handler(event));
    }
    const nlohmann::json payload{{"login", {{"username", userName}, {"password", password}}}};
    const nlohmann::json userResponse{{"user", userName}, {"userid", userId}, {"pic", ""}};
    // Valid password
    {
        const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
        InSequence s;
        EXPECT_CALL(sockComm.GetServer(),
            send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }),
                Truly([&](const std::string& s) {
                    nlohmann::json json = nlohmann::json::parse(s);
                    return authenticator.ValidateJWTToken(json.at("idToken")) != nullptr
                        && json.at("expiresIn")
                        == std::chrono::duration_cast<std::chrono::seconds>(AuthEventHandler::s_tokenExpiration)
                               .count();
                }),
                websocketpp::frame::opcode::text));
        EXPECT_CALL(sockComm.GetServer(),
            send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }), userResponse.dump(),
                websocketpp::frame::opcode::text));
        EXPECT_EQ(PostEventState::handled | PostEventState::consumeEvent, handler(event));
    }
}
