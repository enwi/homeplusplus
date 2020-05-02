#pragma once


#include "../communication/WebsocketChannel.h"
#include "../database/DBHandler.h"

class ProfileSocketHandler
{
public:
  ProfileSocketHandler(DBHandler& dbHandler, const Authenticator& authenticator) : m_dbHandler(&dbHandler), m_authenticator(&authenticator) {}

  PostEventState operator()(const WebsocketChannel::EventVariant& event, WebsocketChannel& channel);

  PostEventState HandleSocketMessage(const Events::SocketMessageEvent& event, WebsocketChannel& channel);

  bool ValidatePassword(const int64_t userid, const std::string& password);

  public:
  static constexpr const char* s_checkPassword = "CHECK_PW";
  static constexpr const char* s_changePassword = "CHANGE_PW";
  static constexpr const char* s_changePicture = "CHANGE_PICTURE";
  static constexpr const char* s_getPicture = "GET_PICTURE";
  static constexpr const char* s_getUser = "GET_USER";

  private:
  DBHandler* m_dbHandler;
  const Authenticator* m_authenticator;
};
