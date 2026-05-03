#pragma once
#include <JuceHeader.h>

/**
 * HttpServer - 負責接收並解析來自網頁端的 HTTP POST 指令
 */
class HttpServer : public juce::Thread
{
public:
    HttpServer(juce::Label& statusLabel);
    ~HttpServer() override;

    void run() override;

private:
    void updateStatus(const juce::String& text, juce::Colour color);
    juce::Label& label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HttpServer)
};
