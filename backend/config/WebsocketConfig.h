#ifndef WEBSOCKET_CONFIG_H
#define WEBSOCKET_CONFIG_H

#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_

#include <websocketpp/server.hpp>

#ifndef C_PLUS_PLUS_TESTING

#include <websocketpp/config/asio_no_tls.hpp>

namespace WebsocketConfig
{
    // Config with disabled payload logging
    struct config : public websocketpp::config::asio
    {
        static const websocketpp::log::level alog_level = websocketpp::log::alevel::none;
    };
    using server = websocketpp::server<config>;
} // namespace WebsocketConfig

#else

#include "test/mocks/MockWebsocketServer.h"

namespace WebsocketConfig
{
    struct config
    {};
    using server = MockWebsocketServer;
} // namespace WebsocketConfig

#endif

#endif