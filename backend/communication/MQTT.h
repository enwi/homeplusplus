#pragma once

#include <absl/container/node_hash_map.h>
#include <absl/hash/hash.h>

#include "MQTTClient.h"

class MQTT
{
public:
    MQTT();
    ~MQTT();

    ///
    /// \brief Subscribe to a specific topic and get published messages on the given callback
    ///
    /// \param topic The topic to subscribe
    /// \param messageCallback The callback that gets called when a message is published to the specified topic
    /// \param ip The ip address of the broker
    /// \param port The port of the broker
    /// \param qos The quality of service for the subscription
    std::future<std::vector<int>> Subscribe(const std::string& topic, const MQTTClient::Callback& messageCallback,
        const std::string& ip, int port = 1883, int qos = 0);

    ///
    /// \brief Publish the given message/payload on the given topic
    ///
    /// \param topic The topic to publish the message on
    /// \param payload The payload to publish
    /// \param ip The ip address of the broker
    /// \param port The port of the broker
    /// \param qos The quality of service of the message
    /// \param retain True to have the message retained by the broker
    std::future<bool> Publish(const std::string& topic, const std::string& payload, const std::string& ip,
        int port = 1883, int qos = 0, bool retain = false);

    ///
    ///\brief Get a specific client
    ///
    /// \param ip The ip address of the broker
    /// \param port The port of the broker
    MQTTClient& GetClient(const std::string& ip, int port = 1883);

private:
    struct ClientConnection
    {
        std::string ip;
        int port;

        bool operator==(const ClientConnection& rhs) const { return ip == rhs.ip && port == rhs.port; }

        template <typename H>
        friend H AbslHashValue(H h, const ClientConnection& c)
        {
            return H::combine(std::move(h), c.ip, c.port);
        }
    };

private:
    ///
    /// \brief Maps connections to clients
    ///
    /// node_hash_map is used, because MQTTClient is not copyable
    absl::node_hash_map<ClientConnection, MQTTClient> mClients;
};
