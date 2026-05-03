#include <JuceHeader.h>
#include "Source/MainWindow.h"

/**
 * MusiCodeEngineApp - 引擎應用程式入口
 * 負責管理全域生命週期與主視窗。
 */
class MusiCodeEngineApp : public juce::JUCEApplication
{
public:
    MusiCodeEngineApp() {}
    
    const juce::String getApplicationName() override       { return "MusiCodeEngine"; }
    const juce::String getApplicationVersion() override    { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise(const juce::String& commandLine) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
    }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

// 啟動應用程式
START_JUCE_APPLICATION(MusiCodeEngineApp)
