// halDeckMIDI.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <chrono>
#include <cstdlib>
#include <halDeckMIDIBackend.h>
#include <iostream>
#include <thread>
#include <libremidi/libremidi.hpp>
#include <atlstr.h>
#include <filesystem>
#include <thread>

using namespace std::string_literals;
namespace fs = std::filesystem;

std::string hexStr(const uint8_t* data, int len);
void send_key_input(const WORD& vk);
void send_key_input(const std::vector<WORD>& vks);
std::string GetModulePath();

int main()
try
{
	const auto exe_path = GetModulePath();
	const fs::path exe_dir = fs::path(exe_path).parent_path();
	const fs::path config_path = exe_dir / "config.toml";
	auto backend_maybe = halDeckMIDIBackend::create("127.0.0.1", 7001, config_path.string());
	if (backend_maybe.has_value() == false)
	{
		std::cerr << backend_maybe.error() << '\n';
		return EXIT_FAILURE;
	}
	auto backend = backend_maybe.value();

	// Set the configuration of our MIDI port
	// Note that the callback will be invoked from a separate thread,
	// it is up to you to protect your data structures afterwards.
	// For instance if you are using a GUI toolkit, don't do GUI actions
	// in that callback !
	auto my_callback = [&](const libremidi::message& message)
		{
			const auto size = message.size();
			const auto data = message.bytes.data();

			std::cout << "Received message: ";
			std::cout << hexStr(data, size) << '\n';

			if (size == 0)
				return;
			if (data[0] == 0xB0) // Control Change
			{
				const auto key = data[1];
				const auto value = data[2];

				if (key == 0x00) // Volume
				{
					const auto volume = value / 127.0f;
					backend.set_volume(volume);
				}

				if (key == 0x29 && value > 0) // Play
				{
					std::cout << "Play\n";
					send_key_input(VK_MEDIA_PLAY_PAUSE);
				}
				if (key == 0x2A && value > 0) // Stop
				{
					std::cout << "Stop\n";
					send_key_input(VK_MEDIA_STOP);
				}
				if (key == 0x2C && value > 0) // Next
				{
					std::cout << "Next\n";
					send_key_input(VK_MEDIA_NEXT_TRACK);
				}
				if (key == 0x2B && value > 0) // Previous
				{
					std::cout << "Previous\n";
					send_key_input(VK_MEDIA_PREV_TRACK);
				}
				if (key == 0x3B && value > 0) // Right Desktop
				{
					std::cout << "Right Desktop\n";
					const std::vector<WORD> vks{ VK_LWIN, VK_CONTROL, VK_RIGHT };
					send_key_input(vks);
				}
				if (key == 0x3A && value > 0) // Left Desktop
				{
					std::cout << "Left Desktop\n";
					const std::vector<WORD> vks{ VK_LWIN, VK_CONTROL, VK_LEFT };
					send_key_input(vks);
				}
				if (key == 0x3C && value > 0) // Toggle Pin Window
				{
					std::cout << "Toggle Pin Window\n";
					std::thread([&backend]()
					{
						auto result = backend.toggle_pin_active_window();
						if (result.has_value() == false)
						{
							std::cerr << result.error() << '\n';
						}
						auto [str_stdout, str_stderr, exit_code] = result.value();
						std::cout << "stdout: " << str_stdout << '\n';
						std::cout << "stderr: " << str_stderr << '\n';
						std::cout << "exit_code: " << exit_code << '\n';
					}).detach();
				}
			}
		};

	// Create the midi object
	libremidi::midi_in midi_in{
		libremidi::input_configuration{.on_message = my_callback}
	};

	for (auto&& api : libremidi::available_apis())
	{
		const std::string_view api_name = libremidi::get_api_display_name(api);
		std::cout << "Displaying ports for: " << api_name << std::endl;

		// On Windows 10, apparently the MIDI devices aren't exactly available as soon as the app open...
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		libremidi::observer midi_obs{ {}, libremidi::observer_configuration_for(api) };
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

std::string hexStr(const uint8_t* data, int len)
{
	std::stringstream ss;
	ss << std::hex;

	for (int i(0); i < len; ++i)
		ss << std::setw(2) << std::setfill('0') << (int)data[i];

	return ss.str();
}

void send_key_input(const std::vector<WORD>& vks)
{
	std::vector<INPUT> ips;
	for (auto&& vk : vks)
	{
		INPUT ip;
		ip.type = INPUT_KEYBOARD;
		ip.ki.wVk = vk;
		ip.ki.dwFlags = 0;
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;
		ips.push_back(ip);
	}
	std::vector<WORD> reversed_vks(vks);
	std::ranges::reverse(reversed_vks);
	for (auto&& vk : reversed_vks)
	{
		INPUT ip;
		ip.type = INPUT_KEYBOARD;
		ip.ki.wVk = vk;
		ip.ki.dwFlags = KEYEVENTF_KEYUP;
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;
		ips.push_back(ip);
	}
	SendInput(ips.size(), ips.data(), sizeof(INPUT));
}

void send_key_input(const WORD& vk)
{
	std::vector<WORD> vks;
	vks.push_back(vk);
	send_key_input(vks);
}

// 実行ファイルパス取得
std::string GetModulePath()
{
	// 返却値
	CString res = static_cast<LPCTSTR>(nullptr);
	// フルパスを取得
	if (TCHAR path[_MAX_PATH]; ::GetModuleFileName(nullptr, path, _MAX_PATH) != 0)
	{
		// パス、ドライブ名、ディレクトリ名、ファイル名、拡張子
		TCHAR drive[_MAX_PATH], dir[_MAX_PATH], file[_MAX_PATH], ext[_MAX_PATH];
		// ファイルパスを分割
		::_tsplitpath_s(path, drive, _MAX_PATH, dir, _MAX_PATH, file, _MAX_PATH, ext, _MAX_PATH);
		// ドライブ名とディレクトリ名を結合
		res = ::PathCombine(path, drive, dir);
	}

	std::string path_str = CStringA(res).GetBuffer();
	return path_str;
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
