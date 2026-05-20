#pragma once
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

/**
 * MidiController - 負責 MIDI 設備管理、軌道路由與訊號監控。
 * 遵循 TE_Cookbook.html 的正確操作順序。
 */
class MidiController : public juce::ChangeListener,
                       public juce::MidiInputCallback,
                       private juce::Timer
{
public:
    MidiController(tracktion_engine::Edit& edit, tracktion_engine::Engine& engine);
    ~MidiController() override;

    /** 獲取目前所有可用的 MIDI 輸入清單 (JSON 格式) */
    juce::var getMidiInputsAsJson();

    /** 
     * 設定特定軌道的輸入源
     * @param trackID 軌道唯一 ID
     * @param deviceName 設備名稱 (如 "iCON iKeyboard 4", "All MIDI Ins")
     */
    bool setTrackInput(tracktion_engine::EditItemID trackID, const juce::String& deviceName);

    /** 更新設備列表 (當 AudioDeviceSelector 變更時呼叫) */
    void refreshDevices();

    /** 註冊一個回呼，當有 MIDI 訊號強度更新時通知 (30Hz) */
    void setSignalLevelCallback(std::function<void(tracktion_engine::EditItemID trackID, float level)> callback)
    {
        signalLevelCallback = callback;
    }

    // 實作 juce::MidiInputCallback
    void handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message) override;

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void timerCallback() override;

    tracktion_engine::Edit& edit;
    tracktion_engine::Engine& engine;

    std::function<void(tracktion_engine::EditItemID trackID, float level)> signalLevelCallback;

    // 快取上一次的訊號強度，減少 IPC 負擔 (使用 ID 作為 Key)
    std::map<tracktion_engine::EditItemID, float> lastSignalLevels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiController)
};
