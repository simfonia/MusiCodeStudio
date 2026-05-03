#pragma once
#include <JuceHeader.h>
#include "HttpServer.h"

/**
 * MainWindow - 引擎的主視窗，目前僅包含狀態顯示標籤與 HttpServer 實例
 */
class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name);
    void closeButtonPressed() override;

private:
    juce::Label statusLabel;
    std::unique_ptr<HttpServer> server;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};
