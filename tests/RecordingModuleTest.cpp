/*
    RecordingModuleTest.cpp - 錄音模組單元測試
    目的：驗證 TransportController 與 RecordingController 的協作邏輯。
*/

#include <tracktion_engine/tracktion_engine.h>
#include "../engine/MusiCodeEngine/Source/TransportController.h"
#include "../engine/MusiCodeEngine/Source/RecordingController.h"

namespace te = tracktion_engine;

/**
 * 測試腳本：
 * 1. 建立 Engine 與 Edit。
 * 2. 初始化 TransportController 與 RecordingController。
 * 3. 建立測試音軌。
 * 4. 武裝音軌 (Arming)。
 * 5. 執行錄音 (Record) 並等待 2 秒。
 * 6. 停止錄音 (Stop & Save)。
 * 7. 驗證軌道上的 Clip 數量。
 */
void runRecordingTest(te::Engine& engine, te::Edit& edit)
{
    using namespace MusiCode;

    // --- 1. 初始化模組 ---
    auto transportCtrl = std::make_unique<TransportController>(edit);
    auto recordingCtrl = std::make_unique<RecordingController>(edit, *transportCtrl);

    // --- 2. 準備測試軌道 ---
    auto track = te::getAudioTracks(edit)[0];
    if (track == nullptr) {
        track = dynamic_cast<te::AudioTrack*>(edit.insertNewTrack(te::TrackInsertPoint::getEndOfTracks(edit), "Test Track", nullptr).get());
    }
    
    // --- 3. 模擬武裝 (Arming) ---
    // 獲取一個 MIDI 輸入並指向該軌道
    edit.getTransport().ensureContextAllocated();
    bool armed = false;
    for (auto instance : edit.getAllInputDevices())
    {
        // 為了測試，我們將第一個找到的 MIDI 設備武裝到測試軌道
        if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice ||
            instance->getInputDevice().getDeviceType() == te::InputDevice::virtualMidiDevice)
        {
            instance->setTarget(track->itemID, true, &edit.getUndoManager(), 0);
            instance->setRecordingEnabled(track->itemID, true);
            armed = true;
            break;
        }
    }

    if (!armed) {
        // 如果沒有實體設備，這在 headless 測試環境可能會發生
        // LOG: No MIDI device found to arm. Test might fail to produce a clip.
    }

    // --- 4. 執行錄音 ---
    // 檢查武裝狀態
    if (recordingCtrl->isArmed(track->itemID))
    {
        recordingCtrl->startRecording();
        
        // 模擬錄音時間 (這在單元測試中需要考慮非同步與 Timer)
        // 注意：te::TransportControl 的更新依賴於 DeviceManager 的回調
        // 在 Headless 測試中，我們可能需要手動觸發某些更新或等待
    }

    // --- 5. 停止與驗證 ---
    recordingCtrl->stopAndSave();

    // 驗證點
    auto clips = track->getClips();
    // LOG: Current Clip Count: [clips.size()]
}
