#include "MQTTClient.h"

#include <mosquitto.h>

#include "../api/Resources.h"
#include "../utility/Logger.h"

MQTTClient::MQTTClient(const std::string& ip, int port)
{
    m_mosq = mosquitto_new("Home++", false, this);
    mosquitto_connect_callback_set(m_mosq, OnConnectWrapper);
    mosquitto_disconnect_callback_set(m_mosq, OnDisconnectWrapper);
    mosquitto_publish_callback_set(m_mosq, OnPublishWrapper);
    mosquitto_message_callback_set(m_mosq, OnMessageWrapper);
    mosquitto_subscribe_callback_set(m_mosq, OnSubscribeWrapper);
    mosquitto_unsubscribe_callback_set(m_mosq, OnUnsubscribeWrapper);
    mosquitto_log_callback_set(m_mosq, OnLogWrapper);
    mosquitto_connect(m_mosq, ip.c_str(), port, 60);

    m_threadRunning = true;
    m_thread = std::thread(&MQTTClient::Loop, this);

    Res::Logger().Debug("MQTTClient", "Client for broker " + ip + ":" + std::to_string(port) + " created");
}

MQTTClient::~MQTTClient()
{
    if (m_threadRunning)
    {
        m_threadRunning = false;
        if (m_thread.joinable())
        {
            // Wait until the thread finishes if it is still running
            m_thread.join();
        }
    }
    mosquitto_destroy(m_mosq);
}

bool MQTTClient::isSubscribed(const std::string& topic)
{
    return std::find(activeSubscriptions.begin(), activeSubscriptions.end(), topic) != activeSubscriptions.end();
}

std::future<std::vector<int>> MQTTClient::Subscribe(const std::string& topic, const Callback& messageCallback, int qos)
{
    Res::Logger().Debug("MQTTClient", "New subscription for topic: " + topic);
    std::promise<std::vector<int>> promise;
    auto future = promise.get_future();
    std::lock_guard<std::mutex> guard(m_mutex);

    if (!isSubscribed(topic))
    {
        int messageId;
        handleErrorCode(mosquitto_subscribe(m_mosq, &messageId, topic.c_str(), qos));
        subscriptions.push_back(std::make_pair(topic, messageCallback));
        activeSubscriptions.push_back(topic);

        newSubscriptions.push_back(std::make_pair(messageId, std::move(promise)));
    }
    else
    {
        subscriptions.push_back(std::make_pair(topic, messageCallback));

        promise.set_value(std::vector<int> {0});
    }

    return future;
}

std::future<bool> MQTTClient::Publish(const std::string& topic, const std::string& payload, int qos, bool retain)
{
    int messageId;
    handleErrorCode(mosquitto_publish(m_mosq, &messageId, topic.c_str(), payload.size(), payload.c_str(), qos, retain));

    std::promise<bool> promise;
    auto future = promise.get_future();

    std::lock_guard<std::mutex> guard(m_mutex);
    newPublishes.push_back(std::make_pair(messageId, std::move(promise)));

    return future;
}

void MQTTClient::handleErrorCode(int code)
{
    if (code != MOSQ_ERR_NO_CONN && code != MOSQ_ERR_SUCCESS)
    {
        throw std::runtime_error(mosquitto_strerror(code));
    }
}

bool MQTTClient::topicMatches(const std::string& subscription, const std::string& topic)
{
    bool result = false;
    // mosquitto_topic_matches_sub2(subscription.c_str(), subscription.length(), topic.c_str(), topic.length(),
    // &result);
    mosquitto_topic_matches_sub(subscription.c_str(), topic.c_str(), &result);
    return result;
}

void MQTTClient::Loop()
{
    Res::Logger().Debug("MQTTClient", "Thread started");
    // std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while (m_threadRunning)
    {
        // std::this_thread::sleep_until(start + s_checkInterval);
        int code = mosquitto_loop(m_mosq, -1, 1);
        // handleReturnCode(code);
        if (code)
        {
            mosquitto_reconnect(m_mosq);
        }
        // start = std::chrono::steady_clock::now();
    }
    Res::Logger().Debug("MQTTClient", "Thread stopped");
}

void MQTTClient::OnConnect(int returnCode) {}

void MQTTClient::OnDisconnect(int reason) {}

void MQTTClient::OnPublish(int messageId)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    auto ite = std::find_if(
        newPublishes.begin(), newPublishes.end(), [&](const auto& cmp) { return cmp.first == messageId; });
    if (ite != newPublishes.end())
    {
        ite->second.set_value(true);
        newPublishes.erase(ite);
    }
}

void MQTTClient::OnMessage(int messageId, std::string topic, std::string paylaod, int qos, bool retain)
{
    // Res::Logger().Info("MQTTClient", "New message received " + topic + " : " + paylaod);
    std::vector<std::reference_wrapper<Callback>> callbacks;
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        for (std::pair<std::string, Callback>& subscription : subscriptions)
        {
            if (topicMatches(subscription.first, topic))
            {
                callbacks.push_back(std::ref(subscription.second));
            }
        }
    }

    for (Callback& callback : callbacks)
    {
        if (callback(messageId, topic, paylaod, qos, retain))
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            auto ite = std::find_if(
                subscriptions.begin(), subscriptions.end(), [&](auto& cmp) { return &cmp.second == &callback; });
            if (ite != subscriptions.end())
            {
                std::string subscribedTopic = ite->first;
                subscriptions.erase(ite);

                ite = std::find_if(subscriptions.begin(), subscriptions.end(),
                    [&](auto& cmp) { return cmp.first == subscribedTopic; });
                if (ite == subscriptions.end())
                {
                    auto aite = std::find(activeSubscriptions.begin(), activeSubscriptions.end(), subscribedTopic);
                    activeSubscriptions.erase(aite);
                    mosquitto_unsubscribe(m_mosq, NULL, subscribedTopic.c_str());
                }
            }
        }
    }
}

void MQTTClient::OnSubscribe(int messageId, std::vector<int> qos)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    auto ite = std::find_if(
        newSubscriptions.begin(), newSubscriptions.end(), [&](const auto& cmp) { return cmp.first == messageId; });
    if (ite != newSubscriptions.end())
    {
        ite->second.set_value(qos);
        newSubscriptions.erase(ite);
    }
}

void MQTTClient::OnUnsubscribe(int messageId) {}

void MQTTClient::OnLog(int level, std::string str) {}

void MQTTClient::OnConnectWrapper(struct mosquitto* mosq, void* userdata, int rc)
{
    class MQTTClient* m = (class MQTTClient*)userdata;
    m->OnConnect(rc);
};

void MQTTClient::OnDisconnectWrapper(struct mosquitto* mosq, void* userdata, int rc)
{
    class MQTTClient* m = (class MQTTClient*)userdata;
    m->OnDisconnect(rc);
};

void MQTTClient::OnPublishWrapper(struct mosquitto* mosq, void* userdata, int messageId)
{
    class MQTTClient* m = (class MQTTClient*)userdata;
    m->OnPublish(messageId);
};

void MQTTClient::OnMessageWrapper(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message)
{
    class MQTTClient* m = (class MQTTClient*)userdata;
    m->OnMessage(message->mid, std::string(message->topic),
        std::string(reinterpret_cast<const char*>(message->payload), message->payloadlen), message->qos,
        message->retain);
};

void MQTTClient::OnSubscribeWrapper(
    struct mosquitto* mosq, void* userdata, int messageId, int qos_count, const int* granted_qos)
{
    class MQTTClient* m = (class MQTTClient*)userdata;
    m->OnSubscribe(messageId, std::vector<int>(granted_qos, granted_qos + qos_count));
};

void MQTTClient::OnUnsubscribeWrapper(struct mosquitto* mosq, void* userdata, int messageId)
{
    class MQTTClient* m = (class MQTTClient*)userdata;
    m->OnUnsubscribe(messageId);
};

void MQTTClient::OnLogWrapper(struct mosquitto* mosq, void* userdata, int level, const char* str)
{
    class MQTTClient* m = (class MQTTClient*)userdata;
    m->OnLog(level, std::string(str));
};
