#pragma once

#include "../communication/Authenticator.h"
#include "../communication/WebsocketCommunication.h"
#include "../database/DBHandler.h"

class AuthEventHandler
{
public:
    AuthEventHandler(DBHandler& dbHandler, WebsocketCommunication& sockComm, const Authenticator& authenticator);

    // Executes handler
    PostEventState operator()(const WebsocketCommunication::EventVariant& e);

    PostEventState HandleSocketMessage(const Events::SocketMessageEvent& event);

    std::string ValidateLogin(const std::string& username, const std::string& password);

public:
    static constexpr std::chrono::system_clock::duration s_tokenExpiration = std::chrono::hours(2);

private:
    DBHandler* m_dbHandler;
    WebsocketCommunication* m_sockComm;
    const Authenticator* m_authenticator;
};
