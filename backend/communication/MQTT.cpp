#include "MQTT.h"

#include <mosquitto.h>

#include "../api/Resources.h"
#include "../utility/Logger.h"

MQTT::MQTT() : mClients {}
{
    mosquitto_lib_init();
    // Res::Logger().Info("MQTT", "Initialized mosquitto lib");
}

MQTT::~MQTT()
{
    mClients.clear();
    mosquitto_lib_cleanup();
    // Res::Logger().Info("MQTT", "Cleaned up mosquitto lib");
}

std::future<std::vector<int>> MQTT::Subscribe(
    const std::string& topic, const MQTTClient::Callback& messageCallback, const std::string& ip, int port, int qos)
{
    ClientConnection connection {ip, port};

    if (!mClients.contains(connection))
    {
        mClients.emplace(std::piecewise_construct, std::forward_as_tuple(connection), std::forward_as_tuple(ip, port));
    }

    return mClients.find(connection)->second.Subscribe(topic, messageCallback, qos);
}

std::future<bool> MQTT::Publish(
    const std::string& topic, const std::string& payload, const std::string& ip, int port, int qos, bool retain)
{
    ClientConnection connection {ip, port};

    if (!mClients.contains(connection))
    {
        mClients.emplace(std::piecewise_construct, std::forward_as_tuple(connection), std::forward_as_tuple(ip, port));
    }

    return mClients.find(connection)->second.Publish(topic, payload, qos, retain);
}

MQTTClient& MQTT::GetClient(const std::string& ip, int port)
{
    ClientConnection connection {ip, port};

    if (!mClients.contains(connection))
    {
        mClients.emplace(std::piecewise_construct, std::forward_as_tuple(connection), std::forward_as_tuple(ip, port));
    }

    return mClients.find(connection)->second;
}