#pragma once
#include <JuceHeader.h>

#include "AudioEngine.h"

/**
 * HttpServer - 負責接收並解析來自網頁端的 HTTP POST 指令
 */
class HttpServer : public juce::Thread
{
public:
    HttpServer(juce::Label& statusLabel, AudioEngine& engine);
    ~HttpServer() override;

    void run() override;

private:
    void handleRequest(juce::StreamingSocket* client);
    void processCommand(const juce::String& jsonString);
    void updateStatus(const juce::String& text, juce::Colour color);

    // 指令映射系統
    using CommandHandler = std::function<void(const juce::var&)>;
    std::map<juce::String, CommandHandler> commandHandlers;
    void registerHandlers();

    juce::Label& label;
    AudioEngine& audioEngine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HttpServer)
};
