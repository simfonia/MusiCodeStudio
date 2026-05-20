/*
    MidiTo4OscTest.cpp - 依據 TE_Cookbook.html 規範編寫
    目的：驗證從 MIDI 輸入到 4OSC 發聲的正確路由流程 (不影響主專案)
*/

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion_engine;

void setupMidiTo4Osc(te::Engine& engine, te::Edit& edit)
{
    // --- 1. 依照手冊 [2. MIDI 處理] 啟用輸入設備 ---
    auto& dm = engine.getDeviceManager();
    for (auto& midiIn : dm.getMidiInDevices())
    {
        // 關鍵：必須設定 MonitorMode 為 automatic，否則彈奏無聲
        midiIn->setMonitorMode(te::InputDevice::MonitorMode::automatic);
        midiIn->setEnabled(true);
    }

    // --- 2. 依照手冊 [4. 插件管理] 建立 4OSC 插件 ---
    // 使用推薦的 insertPlugin 方式，而非依賴索引
    if (auto track = te::getAudioTracks(edit)[0]) // 假設使用第一個軌道
    {
        // 清除舊插件 (防止干擾)
        track->pluginList.removeEveryPlugin();
        
        // 插入 4OSC
        if (auto p = edit.getPluginCache().createNewPlugin(te::FourOscPlugin::xmlTypeName, {}))
        {
            track->pluginList.insertPlugin(p, 0, nullptr);
            
            // 依照手冊 [API 索引]：4OSC 必須手動設定 filterType 才能確保發聲路徑正確 (某些版本預設靜音)
            if (auto filterParam = p->getAutomatableParameterByID("filterType"))
                filterParam->setParameter(1.0f, juce::sendNotification); // 設為 Lowpass
        }

        // --- 3. 依照手冊 [2. MIDI 處理] 配置路由與監聽 ---
        for (auto instance : edit.getAllInputDevices())
        {
            // 尋找實體 MIDI 設備
            if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice)
            {
                // 關鍵：設定 Target 為該軌道
                instance->setTarget(track->itemID, true, &edit.getUndoManager(), 0);
                
                // 關鍵：必須 setRecordingEnabled(true) 才能啟動監聽流，即便不錄音
                instance->setRecordingEnabled(track->itemID, true);
                
                // 驗證是否成功開啟 LivePlay
                if (instance->isLivePlayEnabled(*track)) {
                    // LOG: MIDI Routing for [Track 0] is now ACTIVE.
                }
            }
        }
    }

    // --- 4. 依照手冊 [3. 播放與傳輸] 啟動傳輸與分配上下文 ---
    auto& transport = edit.getTransport();
    
    // 關鍵：確保音訊處理鏈已正確建立
    transport.ensureContextAllocated();
    
    // 啟動播放 (若需要即時彈奏，Transport 需在 Playing 或準備就緒狀態)
    if (!transport.isPlaying())
        transport.play(false);
}
