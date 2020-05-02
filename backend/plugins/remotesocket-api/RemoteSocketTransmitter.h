#pragma once

#include <string>

#ifdef HOMEPLUSPLUS_REMOTESOCKET
#include <RCSwitch.h>
#endif

class RemoteSocketTransmitter
{
public:
    void Init(int transmitPin);
    void Shutdown();
    // Code: 5 digit 0/1: group + 1 digit 1-4: switch
    void SetSocket(const std::string& code, bool on);

private:
#ifdef HOMEPLUSPLUS_REMOTESOCKET
    RCSwitch m_switch;
#endif
};