#include "MainWindow.h"

MainWindow::MainWindow(juce::String name) 
    : DocumentWindow(name, juce::Colours::white, allButtons)
{
    setUsingNativeTitleBar(true);
    setCentreRelative(0.5f, 0.5f);
    setVisible(true);

    statusLabel.setText("MusiCode Engine: HTTP Ready (9001)", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    statusLabel.setJustificationType(juce::Justification::centred);
    setContentOwned(&statusLabel, false);

    server = std::make_unique<HttpServer>(statusLabel);
    setSize(400, 200);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
