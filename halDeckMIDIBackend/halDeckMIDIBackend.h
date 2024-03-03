#pragma once
#include "pch.h"

#define DLLEXPORT   __declspec( dllexport )

struct Config
{
    std::string vd_path;
};

class DLLEXPORT halDeckMIDIBackend
{
public:
    std::expected<halDeckMIDIBackend, std::string> static create(const unsigned long& address, const int& port, const std::string& config_path);
    std::expected<halDeckMIDIBackend, std::string> static create(const char* address, const int& port, const std::string& config_path);
    void set_volume(const float& volume);
private:
    halDeckMIDIBackend(const unsigned long& address, const int& port) : socket_(IpEndpointName(address, port)) {}
    halDeckMIDIBackend(const char*& address, const int& port) : socket_(IpEndpointName(address, port)) {}
    UdpTransmitSocket socket_;
    static std::expected<Config, std::string> load_config(const std::string& toml_path);
    Config config_;
};
