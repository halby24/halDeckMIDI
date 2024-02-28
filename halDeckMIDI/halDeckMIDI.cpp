// halDeckMIDI.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>

#include <iostream>
#include <windows.h>

#include <csignal>
#include <iostream>
#include <synchapi.h>

volatile std::sig_atomic_t signal_received = 0;

// シグナルハンドラ関数
void signal_handler(int signal) {
    signal_received = 1;
}

void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

int main() {
    std::signal(SIGTERM, signal_handler);

    HMIDIIN hMidiDevice = nullptr;
    UINT deviceId = 1; // 0は通常デフォルトのMIDIデバイスを指します。利用可能なデバイスのリストはmidiInGetDevCaps関数で取得できます。

    // MIDIデバイスを開く
    const MMRESULT result = midiInOpen(&hMidiDevice, deviceId, reinterpret_cast<DWORD_PTR>(MidiInProc), 0, CALLBACK_FUNCTION);
    if(result != MMSYSERR_NOERROR) {
        std::cerr << "Failed to open MIDI input device." << std::endl;
        return 1;
    }

    // MIDI入力の受信を開始
    midiInStart(hMidiDevice);
    // SIGTERMを受け取るまでループを続ける
    while (!signal_received) {
        std::cin.get(); // プロセスを中断し、シグナルを待つ
    }


    // MIDIデバイスを閉じる
    midiInStop(hMidiDevice);
    midiInClose(hMidiDevice);

    std::cout << "サーバー終了." << std::endl;
    return 0;
}

// MIDI入力コールバック関数
void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    switch(wMsg) {
    case MIM_OPEN:
        std::cout << "MIDI Device Opened" << std::endl;
        break;
    case MIM_CLOSE:
        std::cout << "MIDI Device Closed" << std::endl;
        break;
    case MIM_DATA:
        std::cout << "MIDI Data Received: " << dwParam1 << std::endl;
        break;
    default: // その他のメッセージ
        std::cout << "MIDI Message: " << wMsg << std::endl;
        break;
    }
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
