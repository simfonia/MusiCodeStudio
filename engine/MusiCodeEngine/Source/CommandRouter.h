#pragma once
#include <JuceHeader.h>
#include "AudioEngine.h"

/**
 * CommandRouter - 負責解析 JSON 指令並分發至 AudioEngine
 * 實現代碼重用，支援 HttpServer 與 Native IPC
 */
class CommandRouter
{
public:
    using StatusCallback = std::function<void(const juce::String&, juce::Colour)>;
    using AudioSettingsCallback = std::function<void()>;
    using EventCallback = std::function<void(const juce::String&, const juce::var&)>;

    CommandRouter(AudioEngine& engine, StatusCallback statusCallback);

    /** 解析並執行 JSON 指令 */
    void processCommand(const juce::String& jsonString);
    /** 直接處理解析後的 var 物件 */
    void processCommand(const juce::var& json);

    void setAudioSettingsCallback(AudioSettingsCallback callback) { audioSettingsCallback = callback; }
    void setEventCallback(EventCallback callback) 
    { 
        eventCallback = callback; 
        audioEngine.setTracksChangedCallback([this](const juce::var& json) {
            if (eventCallback != nullptr)
                eventCallback("tracks_list", json);
        });
    }

private:
    void registerHandlers();
    void updateStatus(const juce::String& text, juce::Colour color);

    AudioEngine& audioEngine;
    StatusCallback statusCallback;
    AudioSettingsCallback audioSettingsCallback;
    EventCallback eventCallback;

    using CommandHandler = std::function<void(const juce::var&)>;
    std::map<juce::String, CommandHandler> commandHandlers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CommandRouter)
};
