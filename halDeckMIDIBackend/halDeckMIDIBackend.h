#pragma once
#include "pch.h"

#define DLLEXPORT   __declspec( dllexport )

class DLLEXPORT halDeckMIDIBackend
{
public:
    halDeckMIDIBackend(const unsigned long address, const int port) : socket_(IpEndpointName(address, port)) {}
    void set_volume(float volume);
private:
    UdpTransmitSocket socket_;
};
