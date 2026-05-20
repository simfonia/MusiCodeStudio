#include "MainWindow.h"
#include "TrackManager.h"

//==============================================================================
MainComponent::MainComponent(AudioEngine& engine)
    : audioEngine(engine)
{
    // 1. 初始化狀態標籤
    statusLabel.setText("MusiCodeStudio: Ready", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    statusLabel.setColour(juce::Label::backgroundColourId, juce::Colours::lightgrey.withAlpha(0.5f));
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    // 2. 初始化指令路由中心
    router = std::make_unique<CommandRouter>(engine, [this](const juce::String& text, juce::Colour color) {
        statusLabel.setText(text, juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, color);
    });

    // 注入 show_audio_settings 指令處理
    router->setAudioSettingsCallback([this]() { showAudioSettings(); });
    router->setEventCallback([this](const juce::String& name, const juce::var& data) {
        sendEventToJS(name, data);
    });

    // 4. 初始化 MIDI 訊號回傳
    audioEngine.getMidiController().setSignalLevelCallback([this](tracktion_engine::EditItemID trackID, float level) {
        juce::DynamicObject::Ptr data = new juce::DynamicObject();
        data->setProperty("trackID", trackID.toString());
        data->setProperty("level", level);
        sendEventToJS("midi_signal", data.get());
    });

    // 5. 初始化 WebView2 瀏覽器 (啟用原生整合)
    webBrowser = std::make_unique<juce::WebBrowserComponent>(
        juce::WebBrowserComponent::Options{}
            .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
            .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2()
                .withUserDataFolder(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                    .getChildFile("MusiCodeStudio")
                                    .getChildFile("WebView2_Cache")))
            .withNativeIntegrationEnabled(true)
            .withNativeFunction("postToBackend", [this](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
                if (args.size() > 0)
                    router->processCommand(args[0]);
                completion(juce::var("OK"));
            })
    );
    addAndMakeVisible(webBrowser.get());
    
    // 載入 Vite 開發伺服器網址
    webBrowser->goToURL("http://localhost:5173");
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    statusLabel.setBounds(area.removeFromTop(30));
    if (webBrowser != nullptr)
        webBrowser->setBounds(area);
}

void MainComponent::showAudioSettings()
{
    auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        audioEngine.getEngine().getDeviceManager().deviceManager,
        1, 256, // 輸入腳位
        1, 256, // 輸出腳位
        true,   // 顯示 MIDI 輸入
        true,   // 顯示 MIDI 輸出
        true,   // 顯示頻道選擇
        false   // 隱藏進階選項
    );

    selector->setSize(500, 450);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(selector.release());
    options.dialogTitle = "Audio Settings";
    options.dialogBackgroundColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = false; // 關鍵：停用原生標題列，改用 JUCE 自繪標題列，避開座標偏移 Bug
    options.resizable = false;
    options.componentToCentreAround = this;

    options.launchAsync();
}

void MainComponent::sendEventToJS(const juce::String& eventName, const juce::var& data)
{
    if (webBrowser != nullptr)
    {
        juce::DynamicObject::Ptr eventObj = new juce::DynamicObject();
        eventObj->setProperty("type", eventName);
        eventObj->setProperty("detail", data);
        
        juce::String json = juce::JSON::toString(eventObj.get());
        
        // 透過 CustomEvent 傳遞給前端 window
        juce::String js = "window.dispatchEvent(new CustomEvent('MusiCodeEngineEvent', { detail: " + json + " }));";
        
        juce::MessageManager::callAsync([this, js]() {
            if (webBrowser != nullptr)
                webBrowser->goToURL("javascript:" + js);
        });
    }
}

//==============================================================================
MainWindow::MainWindow(juce::String name) 
    : DocumentWindow("MusiCodeStudio", juce::Colours::white, allButtons)
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);

    // 1. 同步初始化音訊引擎 (關鍵：確保在 UI 載入前完成)
    audioEngine = std::make_unique<AudioEngine>();
    
    // 2. 建立內容容器
    mainComponent = std::make_unique<MainComponent>(*audioEngine);
    setContentOwned(mainComponent.get(), false);

    // 3. 初始化 HTTP 伺服器 (連回容器內的 Label)
    server = std::make_unique<HttpServer>(mainComponent->getStatusLabel(), *audioEngine);

    setFullScreen(true);
    setVisible(true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
