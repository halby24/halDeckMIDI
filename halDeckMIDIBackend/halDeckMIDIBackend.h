#pragma once
#include "pch.h"

#define DLLEXPORT   __declspec( dllexport )

struct Config
{
    std::string vd_path;
};

struct ProcessResult
{
    std::string str_stdout;
    std::string str_stderr;
    DWORD exit_code;
};

class DLLEXPORT halDeckMIDIBackend
{
public:
    std::expected<halDeckMIDIBackend, std::string> static create(const unsigned long& address, const int& port, const std::string& config_path);
    std::expected<halDeckMIDIBackend, std::string> static create(const char* address, const int& port, const std::string& config_path);
    void set_volume(const float& volume);
    std::expected<ProcessResult, std::string> toggle_pin_active_window() const;
private:
    halDeckMIDIBackend(const unsigned long& address, const int& port) : socket_(IpEndpointName(address, port)) {}
    halDeckMIDIBackend(const char*& address, const int& port) : socket_(IpEndpointName(address, port)) {}
    UdpTransmitSocket socket_;
    static std::expected<Config, std::string> load_config(const std::string& toml_path);
    std::expected<ProcessResult, std::string> execute_vd(const std::string& args) const;
    Config config_;
};
