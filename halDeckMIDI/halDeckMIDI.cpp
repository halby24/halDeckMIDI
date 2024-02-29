// halDeckMIDI.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <libremidi/libremidi.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <map>
#include <thread>

std::string hexStr(const uint8_t *data, int len);

int main()
try
{
    // Set the configuration of our MIDI port
    // Note that the callback will be invoked from a separate thread,
    // it is up to you to protect your data structures afterwards.
    // For instance if you are using a GUI toolkit, don't do GUI actions
    // in that callback !
    auto my_callback = [](const libremidi::message& message) {
        std::cout << "Received message: ";
        std::cout << hexStr(message.bytes.data(), message.size()) << '\n';
    };

    // Create the midi object
    libremidi::midi_in midi_in{
        libremidi::input_configuration{ .on_message = my_callback }
    };

    for (auto& api : libremidi::available_apis())
    {
        std::string_view api_name = libremidi::get_api_display_name(api);
        std::cout << "Displaying ports for: " << api_name << std::endl;

        // On Windows 10, apparently the MIDI devices aren't exactly available as soon as the app open...
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        libremidi::observer midi_obs{{}, libremidi::observer_configuration_for(api)};
        {
            // Check inputs.
            auto ports = midi_obs.get_input_ports();
            std::cout << ports.size() << " MIDI input sources:\n";
            for (auto&& port : ports)
            {
                auto name = port.port_name;
                std::cout << " - " << name << '\n';
                if (name.find("nanoKONTROL2") != std::string::npos)
                {
                    std::cout << "  - nanoKONTROL2 found\n";

                    // Open a given midi port.
                    // The argument is a libremidi::input_port gotten from a libremidi::observer.
                    midi_in.open_port(port);
                    std::cin.get();
                }
            }
        }

        {
            // Check outputs.
            auto ports = midi_obs.get_output_ports();
            std::cout << ports.size() << " MIDI output sinks:\n";
            for (auto&& port : ports)
                std::cout << " - " << port.port_name << '\n';
        }

        std::cout << "\n";
    }
    return 0;
}
catch (const libremidi::midi_exception& error)
{
    std::cerr << error.what() << std::endl;
    return EXIT_FAILURE;
}

std::string hexStr(const uint8_t *data, int len)
{
    std::stringstream ss;
    ss << std::hex;

    for( int i(0) ; i < len; ++i )
        ss << std::setw(2) << std::setfill('0') << (int)data[i];

    return ss.str();
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
