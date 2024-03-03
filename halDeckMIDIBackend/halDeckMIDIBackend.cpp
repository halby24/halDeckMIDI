#include "pch.h"
#include "halDeckMIDIBackend.h"
#include <toml++/toml.hpp>
#include <iostream>

constexpr int output_buffer_size = 1024;

std::expected<halDeckMIDIBackend, std::string> halDeckMIDIBackend::create(const unsigned long& address, const int& port,
                                                                          const std::string& config_path)
{
    halDeckMIDIBackend hdmb(address, port);
    auto config = load_config(config_path);
    if (config.has_value() == false)
    {
        return std::unexpected{config.error()};
    }
    hdmb.config_ = config.value();
    return hdmb;
}

std::expected<halDeckMIDIBackend, std::string> halDeckMIDIBackend::create(const char* address, const int& port,
                                                                          const std::string& config_path)
{
    halDeckMIDIBackend hdmb(address, port);
    auto config = load_config(config_path);
    if (config.has_value() == false)
    {
        return std::unexpected{config.error()};
    }
    hdmb.config_ = config.value();
    return hdmb;
}


void halDeckMIDIBackend::set_volume(const float& volume)
{
    char buffer[output_buffer_size];
    osc::OutboundPacketStream p(buffer, output_buffer_size);

    p << osc::BeginMessage("/1/mastervolume")
        << volume
        << osc::EndMessage;

    std::cout << "Sending message: " << p.Data() << '\n';

    socket_.Send(p.Data(), p.Size());
}

std::expected<Config, std::string> halDeckMIDIBackend::load_config(const std::string& toml_path)
try
{
    // const toml::table tbl = toml::parse_file(toml_path);
    // std::cout << tbl << "\n";
    // return Config{tbl["vd_path"].value_or("")};
    return Config{"vd_path"};
}
catch (const toml::parse_error& err)
{
    return std::unexpected{err.what()};
}
