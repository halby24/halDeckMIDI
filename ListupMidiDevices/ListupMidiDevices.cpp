// ListupMidiDevices.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>

int main() {
    const UINT numDevs = midiInGetNumDevs(); // 利用可能なMIDI入力デバイスの数を取得
    if (numDevs == 0) {
        std::cout << "No MIDI input devices available." << '\n';
        return 0;
    }

    MIDIINCAPS midiInCaps;
    MMRESULT result;

    for (UINT i = 0; i < numDevs; ++i) {
        // 各デバイスの能力情報を取得
        result = midiInGetDevCaps(i, &midiInCaps, sizeof(MIDIINCAPS));
        if (result != MMSYSERR_NOERROR) {
            std::cerr << "Error getting device capabilities for device " << i << '\n';
            continue;
        }

        // デバイス情報を表示
        std::wcout << "Device ID: " << i << '\n';
        std::wcout << "Name: " << midiInCaps.szPname << '\n';
        std::wcout << "MIDI Channels: " << (midiInCaps.wMid == 0 ? L"Unknown" : std::to_wstring(midiInCaps.wMid)) <<
            '\n';
        std::wcout << "Manufacturer ID: " << midiInCaps.wMid << '\n';
        std::wcout << "Product ID: " << midiInCaps.wPid << '\n';
        std::wcout << "Driver Version: " << HIWORD(midiInCaps.vDriverVersion) << "." << LOWORD(midiInCaps.vDriverVersion) <<
            '\n' << '\n';
        std::cout << "\n";
    }

    return 0;
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
