#pragma once
#include <JuceHeader.h>

#include "PluginController.h"
#include "MidiController.h"
#include "TransportController.h"
#include "RecordingController.h"

/**
 * AudioEngine - 封裝 Tracktion Engine 的核心邏輯
 * 負責管理 te::Engine, te::Edit 與各功能控制器。
 */
class AudioEngine 
{
public:
    AudioEngine();
    ~AudioEngine();

    // 播放與錄音控制 (委派給對應 Controller)
    void play();
    void stop();
    void record();
    bool isPlaying() const;
    bool isRecording() const;

    // 參數設置
    void setBpm(double newBpm);
    double getBpm() const;
    void setPluginParameter(juce::String pluginName, juce::String paramID, float newValue);

    // 獲取控制器引用
    MidiController& getMidiController() { return *midiController; }
    MusiCode::TransportController& getTransportController() { return *transportController; }
    MusiCode::RecordingController& getRecordingController() { return *recordingController; }

    /** 建立測試場景 (Track 1, 4OSC, MIDI Clip) */
    void setupTestScene();

    // 獲取內部引用 (供後續音軌管理使用)
    tracktion_engine::Engine& getEngine() { return engine; }
    tracktion_engine::Edit& getEdit() { return *edit; }
    PluginController& getPluginController() { return *pluginController; }

private:
    tracktion_engine::Engine engine;
    std::unique_ptr<tracktion_engine::Edit> edit;
    std::unique_ptr<PluginController> pluginController;
    std::unique_ptr<MidiController> midiController;
    
    // 新增模組化控制器
    std::unique_ptr<MusiCode::TransportController> transportController;
    std::unique_ptr<MusiCode::RecordingController> recordingController;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
