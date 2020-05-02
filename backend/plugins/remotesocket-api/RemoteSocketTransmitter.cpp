#include "RemoteSocketTransmitter.h"

#include <stdexcept>

#include "../api/Resources.h"
#include "../utility/Logger.h"

#ifdef HOMEPLUSPLUS_REMOTESOCKET

#include <wiringPi.h>

void RemoteSocketTransmitter::Init(int transmitPin)
{
    // wiringPiSetup();
    // m_switch.enableTransmit(transmitPin);
}

void RemoteSocketTransmitter::Shutdown()
{
    // m_switch.disableTransmit();
}

void RemoteSocketTransmitter::SetSocket(const std::string& code, bool on)
{
    if (code.size() != 6)
    {
        throw std::invalid_argument("RemoteSocketTransmitter::SetSocket got invalid code");
    }
    const int switchNumber = code.back() - '0';
    if (switchNumber < 1 || switchNumber > 4)
    {
        throw std::invalid_argument("RemoteSocketTransmitter::SetSocket got invalid code");
    }
    const std::string group = code.substr(0, code.size() - 1);
    if (on)
    {
        // m_switch.switchOn(group.c_str(), switchNumber);
    }
    else
    {
        // m_switch.switchOff(group.c_str(), switchNumber);
    }
}
#else

void RemoteSocketTransmitter::Init(int transmitPin) {}

void RemoteSocketTransmitter::Shutdown() {}

void RemoteSocketTransmitter::SetSocket(const std::string& code, bool on)
{
    if (code.size() != 6)
    {
        throw std::invalid_argument("RemoteSocketTransmitter::SetSocket got invalid code");
    }
    const int switchNumber = code.back() - '0';
    if (switchNumber < 1 || switchNumber > 4)
    {
        throw std::invalid_argument("RemoteSocketTransmitter::SetSocket got invalid code");
    }
    const std::string group = code.substr(0, code.size() - 1);
    if (on)
    {
        Res::Logger().Debug("RemotesocketTransmitter", "Switch on");
    }
    else
    {
        Res::Logger().Debug("RemotesocketTransmitter", "Switch off");
    }
}
#endif