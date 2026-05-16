#include "MainWindow.h"

//==============================================================================
MainComponent::MainComponent(AudioEngine& engine)
{
    // 1. 初始化狀態標籤
    statusLabel.setText("MusiCode Engine: Initializing...", juce::dontSendNotification);
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

    // 3. 初始化 WebView2 瀏覽器 (啟用原生整合)
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

//==============================================================================
MainWindow::MainWindow(juce::String name) 
    : DocumentWindow(name, juce::Colours::white, allButtons)
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

    setCentreRelative(0.5f, 0.5f);
    setSize(1280, 800);
    setVisible(true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
