#pragma once

constexpr const char* address = "127.0.0.1";
constexpr int port = 7000;

class halDeckMIDIBackend
{
public:
    halDeckMIDIBackend() : socket_(IpEndpointName(address, port)) {}
    void set_volume(float volume);
private:
    UdpTransmitSocket socket_;
};
