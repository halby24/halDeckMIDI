#include "pch.h"
#include "halDeckMIDIBackend.h"
#include <toml++/toml.hpp>
#include <iostream>
#include <sstream>

using namespace std::string_literals;

constexpr int output_buffer_size = 1024;
std::vector<wchar_t> string_to_wchar(std::string in_str);
std::expected<ProcessResult, std::string> execute(wchar_t* cmd);

std::expected<halDeckMIDIBackend, std::string> halDeckMIDIBackend::create(const unsigned long& address, const int& port,
	const std::string& config_path)
{
	halDeckMIDIBackend hdmb(address, port);
	auto config = load_config(config_path);
	if (config.has_value() == false)
	{
		return std::unexpected{ config.error() };
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
		return std::unexpected{ config.error() };
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

std::expected<ProcessResult, std::string> halDeckMIDIBackend::toggle_pin_active_window() const
{
	const auto hwnd = reinterpret_cast<int>(GetForegroundWindow());
	std::ostringstream oss0;
	oss0 << "/IsWindowHandlePinned:" << hwnd;

	auto result = execute_vd(oss0.str());
	if (result.has_value() == false)
	{
		std::cerr << result.error() << '\n';
	}
	auto [str_stdout, str_stderr, exit_code] = result.value();

	std::ostringstream oss1;
	switch (exit_code)
	{
	case 0:
		oss1 << "/UnPinWindowHandle:" << hwnd;
		break;
	default:
		oss1 << "/PinWindowHandle:" << hwnd;
		break;
	}
	return execute_vd(oss1.str());
}

std::expected<Config, std::string> halDeckMIDIBackend::load_config(const std::string& toml_path)
try
{
	const toml::table tbl = toml::parse_file(toml_path);
	std::cout << tbl << "\n";
	return Config{ tbl["vd_path"].value_or("") };
}
catch (const toml::parse_error& err)
{
	return std::unexpected{ err.what() };
}

std::expected<ProcessResult, std::string> halDeckMIDIBackend::execute_vd(const std::string& args) const
{
	// 起動したいアプリケーションのフルパス(あるいはカレントフォルダーからの相対パス)と、
	// 必要に応じて引数も空白区切りにて付加する。
	std::ostringstream oss;
	oss << config_.vd_path << " " << args;
	auto cmd_vec = string_to_wchar(oss.str());
	const auto cmd = cmd_vec.data();

	return execute(cmd);
}

std::vector<wchar_t> string_to_wchar(std::string in_str)
{
	setlocale(LC_ALL, "Japanese_Japan.932");
	std::vector<wchar_t> out_wstr;
	const size_t newsizew = strlen(in_str.c_str()) + 1;
	size_t convertedChars = 0;
	out_wstr.resize(newsizew);
	mbstowcs_s(&convertedChars, out_wstr.data(), newsizew, in_str.c_str(), _TRUNCATE);
	return out_wstr;
}

void ReadFromPipe(HANDLE hPipe, std::string& output)
{
	DWORD bytesRead;
	char buffer[4096];
	while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0)
	{
		buffer[bytesRead] = '\0'; // null-terminate the string
		output += buffer;
	}
}

std::expected<ProcessResult, std::string> execute(wchar_t* cmd)
{
	HANDLE hStdOutRead{}, hStdOutWrite{};
	HANDLE hStdErrRead{}, hStdErrWrite{};
	SECURITY_ATTRIBUTES sa;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = nullptr;
	sa.bInheritHandle = TRUE;

	if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0) ||
		!CreatePipe(&hStdErrRead, &hStdErrWrite, &sa, 0))
	{
		std::unexpected{ "Create Pipe failed" };
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.hStdError = hStdErrWrite;
	si.hStdOutput = hStdOutWrite;
	si.dwFlags |= STARTF_USESTDHANDLES;
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(nullptr, cmd, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi))
	{
		std::unexpected{ "Create Process failed" };
	}

	CloseHandle(hStdOutWrite);
	CloseHandle(hStdErrWrite);

	std::string stdOutput, stdError;
	ReadFromPipe(hStdOutRead, stdOutput);
	ReadFromPipe(hStdErrRead, stdError);

	// プロセスの終了を待つ
	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exitCode;
	GetExitCodeProcess(pi.hProcess, &exitCode);

	ProcessResult result;
	result.str_stdout = stdOutput;
	result.str_stderr = stdError;
	result.exit_code = exitCode;

	CloseHandle(hStdOutRead);
	CloseHandle(hStdErrRead);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return result;
}
