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
    statusLabel.setColour(juce::Label::backgroundColourId, juce::Colours::lightgrey.withAlpha(0.8f));
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    // 2. 初始化指令路由中心
    router = std::make_unique<CommandRouter>(engine, [this](const juce::String& text, juce::Colour color) {
        statusLabel.setText(text, juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, color);
    });

    // 3. 初始化 Web 控制器 (模組化)
    webController = std::make_unique<MusiCode::WebController>(*router);
    addAndMakeVisible(webController->getBrowser());

    // 4. 注入回呼
    router->setAudioSettingsCallback([this]() { showAudioSettings(); });
    router->setEventCallback([this](const juce::String& name, const juce::var& data) {
        webController->sendEvent(name, data);
    });

    // 5. 初始化 MIDI 訊號回傳 (由 WebController 轉發)
    audioEngine.getMidiController().setSignalLevelCallback([this](tracktion_engine::EditItemID trackID, float level) {
        juce::DynamicObject::Ptr data = new juce::DynamicObject();
        data->setProperty("trackID", trackID.toString());
        data->setProperty("level", level);
        webController->sendEvent("midi_signal", data.get());
    });

    // 載入 UI
    webController->loadURL("http://localhost:5173");
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white); 
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    statusLabel.setBounds(area.removeFromTop(30));
    if (webController != nullptr)
        webController->getBrowser()->setBounds(area);
}

void MainComponent::showAudioSettings()
{
    auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        audioEngine.getEngine().getDeviceManager().deviceManager,
        1, 256, 1, 256, true, true, true, false
    );
    selector->setSize(500, 450);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(selector.release());
    options.dialogTitle = "Audio Settings";
    options.dialogBackgroundColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = false;
    options.resizable = false;
    options.componentToCentreAround = this;
    options.launchAsync();
}

//==============================================================================
MainWindow::MainWindow(juce::String name) 
    : DocumentWindow("MusiCodeStudio", juce::Colours::white, allButtons)
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    audioEngine = std::make_unique<AudioEngine>();
    mainComponent = std::make_unique<MainComponent>(*audioEngine);
    setContentOwned(mainComponent.get(), false);
    server = std::make_unique<HttpServer>(mainComponent->getStatusLabel(), *audioEngine);
    setFullScreen(true);
    setVisible(true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
