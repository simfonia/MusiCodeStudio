#include "CommandRouter.h"

CommandRouter::CommandRouter(AudioEngine& engine, StatusCallback callback)
    : audioEngine(engine), statusCallback(callback)
{
    registerHandlers();
}

void CommandRouter::registerHandlers()
{
    commandHandlers["transport_play"] = [this](const juce::var&) {
        audioEngine.play();
        updateStatus("MusiCode Engine: PLAYING", juce::Colours::green);
    };

    commandHandlers["transport_stop"] = [this](const juce::var&) {
        audioEngine.stop();
        updateStatus("MusiCode Engine: STOPPED", juce::Colours::red);
    };

    commandHandlers["set_bpm"] = [this](const juce::var& params) {
        double bpm = params.getProperty("value", 120.0);
        audioEngine.setBpm(bpm);
        updateStatus("MusiCode Engine: BPM -> " + juce::String(bpm, 1), juce::Colours::blue);
    };

    commandHandlers["show_plugin_window"] = [this](const juce::var& params) {
        int trackIndex = params.getProperty("track", 0);
        audioEngine.getPluginController().showPluginWindow(trackIndex);
        updateStatus("Showing Plugin Window (Track " + juce::String(trackIndex) + ")", juce::Colours::cyan);
    };

    commandHandlers["set_plugin_param"] = [this](const juce::var& params) {
        juce::String pluginName = params.getProperty("pluginName", "4OSC").toString();
        juce::String paramID = params.getProperty("paramID", "").toString();
        float value = params.getProperty("value", 0.5f);
        audioEngine.setPluginParameter(pluginName, paramID, value);
    };
}

void CommandRouter::processCommand(const juce::String& jsonString)
{
    auto json = juce::JSON::parse(jsonString);
    processCommand(json);
}

void CommandRouter::processCommand(const juce::var& json)
{
    if (json.isObject())
    {
        auto action = json.getProperty("action", "").toString();
        auto it = commandHandlers.find(action);
        if (it != commandHandlers.end())
        {
            it->second(json);
        }
        else
        {
            updateStatus("Unknown Action: [" + action + "]", juce::Colours::orange);
        }
    }
}

void CommandRouter::updateStatus(const juce::String& text, juce::Colour color)
{
    if (statusCallback != nullptr)
        statusCallback(text, color);
}
