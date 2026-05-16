#pragma once
#include <JuceHeader.h>
#include "AudioEngine.h"
#include "HttpServer.h"
#include "CommandRouter.h"

/**
 * MainComponent - 承載所有 UI 元素的容器
 */
class MainComponent : public juce::Component
{
public:
    MainComponent(AudioEngine& engine);
    void resized() override;
    juce::Label& getStatusLabel() { return statusLabel; }
    
    // 彈出音訊設定對話框
    void showAudioSettings();

private:
    AudioEngine& audioEngine;
    juce::Label statusLabel;
    std::unique_ptr<juce::WebBrowserComponent> webBrowser;
    std::unique_ptr<CommandRouter> router;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

/**
 * MainWindow - 引擎的主視窗
 */
class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name);
    void closeButtonPressed() override;

private:
    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<HttpServer> server;
    std::unique_ptr<MainComponent> mainComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

