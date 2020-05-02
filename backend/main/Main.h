#ifndef _MAIN_H
#define _MAIN_H
#include <condition_variable>
#include <map>
#include <mutex>

#include "ArgumentParser.h"

#include "../api/DeviceAPI.h"
#include "../api/DeviceRegistry.h"
#include "../communication/Authenticator.h"
#include "../communication/MQTT.h"
#include "../communication/WebsocketCommunication.h"
#include "../database/DBActionSerialize.h"
#include "../database/DBDeviceSerialize.h"
#include "../database/DBHandler.h"
#include "../database/DBRuleSerialize.h"
#include "../events/LoadRequestHandler.h"

/*!
 * \brief The main class, running everything else.
 *
 * Initializes and starts every part of the system.
 * Contains the DBHandler for file access,
 * the NodeCommunication for communication with nodes
 * and the SocketCommunication for communication with the web page.
 */
class Main
{
public:
    /*!
     * \brief Initializes the program with Arguments parsed from the command line.
     *
     * The constructor initializes the Logger and adds INodeMessageHandler%s and ISocketMessageHandler%s,
     * but does open the .csv file or start the listeners yet. Call Run() to start the program.
     */
    explicit Main(const Arguments& args);
    /*!
     * \brief Frees used resources and closes files.
     *
     * Closes the Logger and DBHandler.
     */
    virtual ~Main();
    /*!
     * \brief Adds the DeviceAPI to the main program.
     */
    void AddDeviceAPI(std::unique_ptr<IDeviceAPI>&& api);
    /*!
     * \brief Runs the main program.
     *
     * \retval 0 No error occured, normal exit.
     * \retval 1 Failed to open .csv file.
     *
     * The function blocks until a SHUTDOWN signal is received.
     *
     * Calls NodeCommunication::Start() and SocketCommunication::Start()
     * and waits until the program should stop.
     */
    int Run();
    /*!
     * \brief Stops the main program.
     *
     * Does not block until it is finished, because that would result in a deadlock.
     *
     * If the program is not started, this might cause a future start to be suspended immediately.
     */
    void Stop();

    DBHandler& GetDBHandler() { return m_dbHandler; }
    WebsocketCommunication& GetSocketComm() { return m_socketComm; }
    Authenticator& GetAuthenticator() { return m_authenticator; }
    MQTT& GetMQTT() { return m_mqtt; }
    DeviceStorage& GetDeviceStorage() { return m_deviceReg.GetStorage(); }

private:
    /*!
     * \brief The DBHandler to handle writing and reading nodes to the .csv file.
     */
    DBHandler m_dbHandler;
    /*!
     * \brief Performs user authentication.
     */
    Authenticator m_authenticator;
    /*!
     * \brief Performs MQTT communication and client handling.
     */
    MQTT m_mqtt;
    /*!
     * \brief The SocketCommunication to handle communication with the web server.
     */
    WebsocketCommunication m_socketComm;
    /*!
     * \brief Set to true if the program should stop.
     *
     * If this value is changed, #m_shutdownMutex should be locked
     * and #m_cvShutdown .notify_all() should be called to notify the main thread
     * of the change.
     */
    bool m_shutdown;
    /*!
     * \brief Notified when the program should stop.
     *
     * The main thread calls \c wait() on this and waits until #m_shutdown
     * is true and it is notified by \c notify_all().
     */
    std::condition_variable m_cvShutdown;
    /*!
     * \brief Locked to access m_shutdown from concurrent threads.
     */
    std::mutex m_shutdownMutex;
    DeviceTypeRegistry m_deviceTypes;
    DBDeviceSerialize m_deviceSer;
    EventEmitter<Events::DeviceChangeEvent> m_deviceEvents;
    EventEmitter<Events::DevicePropertyChangeEvent> m_propertyEvents;
    /*!
     * \brief Registry for DeviceAPIs.
     */
    DeviceRegistry m_deviceReg;
    /*!
     * \brief Config dir for the DeviceAPIs.
     */
    std::string m_apiConfigDir;
    /*!
     * \brief Serializes Actions from/to database.
     */
    DBActionSerialize m_actionSer;
    /*!
     * \brief Serializes Rules from/to database.
     */
    DBRuleSerialize m_ruleSer;
};
#endif