#include "pch.h"
#include "halDeckMIDIBackend.h"

#include <iostream>

constexpr int output_buffer_size = 1024;

void halDeckMIDIBackend::set_volume(float volume)
{
    char buffer[output_buffer_size];
    osc::OutboundPacketStream p(buffer, output_buffer_size);

    p << osc::BeginMessage("/1/mastervolume")
        << volume
        << osc::EndMessage;

    std::cout << "Sending message: " << p.Data() << std::endl;

    socket_.Send(p.Data(), p.Size());
}
