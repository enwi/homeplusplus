#pragma once

#include <algorithm>
#include <atomic>
#include <functional>
#include <future>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/// \see https://mosquitto.org/api/files/mosquitto-h.html
class MQTTClient
{
public:
    using Callback = std::function<bool(int, const std::string&, const std::string&, int, bool)>;

public:
    ///
    /// \brief MQTTClient constructor
    ///
    /// \param ip The ip address of the broker
    /// \param port The port of the broker
    MQTTClient(const std::string& ip, int port = 1883);

    ///
    /// \brief MQTTClient destructor
    ///
    ~MQTTClient();

    ///
    /// \brief Subscribe to a specific topic and get published messages on the given callback
    ///
    /// \param topic The topic to subscribe
    /// \param messageCallback The callback that gets called when a message is published to the specified topic
    /// \param qos The quality of service for the subscription
    std::future<std::vector<int>> Subscribe(const std::string& topic, const Callback& messageCallback, int qos = 0);

    ///
    /// \brief Publish the given message/payload on the given topic
    ///
    /// \param topic The topic to publish the message on
    /// \param payload The payload to publish
    /// \param qos The quality of service of the message
    /// \param retain True to have the message retained by the broker
    std::future<bool> Publish(const std::string& topic, const std::string& payload, int qos = 0, bool retain = false);

private:
    ///
    /// \brief Check whether the given topic is already subscribed
    ///
    /// Calls to this function must be locked by m_mutex
    /// \param topic The topic to check
    /// \return True if the topic is subscribed, false if nots
    bool isSubscribed(const std::string& topic);

    ///
    /// \brief Evaluate an error/return code and throw an exception if necessary
    ///
    void handleErrorCode(int code);

    bool topicMatches(const std::string& subscription, const std::string& topic);

    ///
    /// \brief Internal loop handling MQTT connection
    ///
    void Loop();

    ///
    /// \brief Called when the broker sends a CONNACK message in response to a connection.
    ///
    /// \param returnCode The return code of the connection response, one of:
    /// - 0 - success
    /// - 1 - connection refused (unacceptable protocol version)
    /// - 2 - connection refused (identifier rejected)
    /// - 3 - connection refused (broker unavailable)
    /// - 4-255 - reserved for future use
    void OnConnect(int returnCode);
    ///
    /// \brief Called when the broker sends a CONNACK message in response to a connection.
    ///
    /// \param returnCode The return code of the connection response, one of:
    /// - 0 - success
    /// - 1 - connection refused (unacceptable protocol version)
    /// - 2 - connection refused (identifier rejected)
    /// - 3 - connection refused (broker unavailable)
    /// - 4-255 - reserved for future use
    /// \param flags The connect flags
    void OnConnectWithFlags(int returnCode, int flags);
    ///
    /// \brief Called when the broker has received the DISCONNECT command and has disconnected the client
    ///
    /// \param reason The reason why the client has been disconnected. A value of 0 means the client has called
    /// mosquitto_disconnect. Any other value indicates that the disconnect is unexpected.
    void OnDisconnect(int reason);
    ///
    /// \brief Called when a message initiated with mosquitto_publish has been sent to the broker successfully
    ///
    /// \param messageId The message identifier of the sent message
    void OnPublish(int messageId);
    ///
    /// \brief Called when a message is received from the broker
    ///
    /// \param messageId The message identifier of the received message
    /// \param topic The topic on which the message was published
    /// \param payload The payload that was published
    /// \param qos The quality of service of the message
    /// \param retain Whether the message should be retained
    void OnMessage(int messageId, std::string topic, std::string paylaod, int qos, bool retain);
    ///
    /// \brief Called when the broker responds to a subscription request
    ///
    /// \param messageId The message identifier of the subscribe message
    /// \param qos A vector of integers indicating the granted QoS for each of the subscriptions
    void OnSubscribe(int messageId, std::vector<int> qos);
    ///
    /// \brief Called when the broker responds to a unsubscription request
    ///
    /// \param messageId The message identifier of the unsubscribe message
    void OnUnsubscribe(int messageId);
    ///
    /// \brief Called when an event of the library occurrred
    ///
    /// \param level The log message level one of:
    /// - MOSQ_LOG_INFO
    /// - MOSQ_LOG_NOTICE
    /// - MOSQ_LOG_WARNING
    /// - MOSQ_LOG_ERR
    /// - MOSQ_LOG_DEBUG
    /// \param str Log message
    void OnLog(int level, std::string str);

    ///
    /// \brief Wraps the \ref OnConnect function and relays the calls to the correct class
    ///
    static void OnConnectWrapper(struct mosquitto* mosq, void* userdata, int returnCode);
    ///
    /// \brief Wraps the \ref OnConnectWithFlags function and relays the calls to the correct class
    ///
    static void OnConnectWithFlagsWrapper(struct mosquitto* mosq, void* userdata, int returnCode, int flags);
    ///
    /// \brief Wraps the \ref OnDisconnect function and relays the calls to the correct class
    ///
    static void OnDisconnectWrapper(struct mosquitto* mosq, void* userdata, int reason);
    ///
    /// \brief Wraps the \ref OnPublish function and relays the calls to the correct class
    ///
    static void OnPublishWrapper(struct mosquitto* mosq, void* userdata, int messageId);
    ///
    /// \brief Wraps the \ref OnMessage function and relays the calls to the correct class
    ///
    static void OnMessageWrapper(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message);
    ///
    /// \brief Wraps the \ref OnSubscribe function and relays the calls to the correct class
    ///
    static void OnSubscribeWrapper(
        struct mosquitto* mosq, void* userdata, int messageId, int qos_count, const int* granted_qos);
    ///
    /// \brief Wraps the \ref OnUnsubscribe function and relays the calls to the correct class
    ///
    static void OnUnsubscribeWrapper(struct mosquitto* mosq, void* userdata, int messageId);
    ///
    /// \brief Wraps the \ref OnLog function and relays the calls to the correct class
    ///
    static void OnLogWrapper(struct mosquitto* mosq, void* userdata, int level, const char* str);

private:
    /*!
     * \brief Mosquitto client instance
     */
    struct mosquitto* m_mosq;
    /*!
     * \brief Indicates that thread is running (as opposed to single-threaded mode).
     */
    std::atomic_bool m_threadRunning {false};
    std::thread m_thread;
    std::mutex m_mutex;
    std::vector<std::pair<std::string, Callback>> subscriptions;
    std::vector<std::string> activeSubscriptions;
    std::vector<std::pair<int, std::promise<std::vector<int>>>> newSubscriptions;
    std::vector<std::pair<int, std::promise<bool>>> newPublishes;
};
