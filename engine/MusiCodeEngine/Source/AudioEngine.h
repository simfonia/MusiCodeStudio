#pragma once
#include <JuceHeader.h>

#include "PluginController.h"

/**
 * AudioEngine - 封裝 Tracktion Engine 的核心邏輯
 * 負責管理 te::Engine, te::Edit 與 Transport 控制。
 */
class AudioEngine 
{
public:
    AudioEngine();
    ~AudioEngine();

    // 播放控制
    void play();
    void stop();
    bool isPlaying() const;

    // 參數設置
    void setBpm(double newBpm);
    double getBpm() const;
    void setPluginParameter(juce::String pluginName, juce::String paramID, float newValue);

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
