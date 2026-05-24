#include "CommandRouter.h"
#include "TrackManager.h"

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

    commandHandlers["transport_record"] = [this](const juce::var&) {
        audioEngine.record();
        updateStatus("MusiCode Engine: RECORDING...", juce::Colours::orange);
    };

    commandHandlers["set_bpm"] = [this](const juce::var& params) {
        double bpm = params.getProperty("value", 120.0);
        audioEngine.setBpm(bpm);
        updateStatus("MusiCode Engine: BPM -> " + juce::String(bpm, 1), juce::Colours::blue);
    };

    commandHandlers["show_plugin_window"] = [this](const juce::var& params) {
        tracktion_engine::EditItemID trackID;
        
        if (params.hasProperty("trackID"))
            trackID = MusiCode::TrackManager::stringToID(params.getProperty("trackID", "").toString());
        else
            trackID = MusiCode::TrackManager::getIDFromIndex(audioEngine.getEdit(), params.getProperty("track", 0));

        audioEngine.getPluginController().showPluginWindow(trackID);
        updateStatus("Showing Plugin Window (Track: " + trackID.toString() + ")", juce::Colours::cyan);
    };

    commandHandlers["set_plugin_param"] = [this](const juce::var& params) {
        juce::String pluginName = params.getProperty("pluginName", "4OSC").toString();
        juce::String paramID = params.getProperty("paramID", "").toString();
        float value = params.getProperty("value", 0.5f);
        audioEngine.setPluginParameter(pluginName, paramID, value);
    };

    commandHandlers["show_audio_settings"] = [this](const juce::var&) {
        if (audioSettingsCallback != nullptr)
            audioSettingsCallback();
        updateStatus("Opening Audio Settings...", juce::Colours::cyan);
    };

    commandHandlers["get_midi_inputs"] = [this](const juce::var&) {
        auto json = audioEngine.getMidiController().getMidiInputsAsJson();
        if (eventCallback != nullptr)
             eventCallback("midi_inputs_list", json);
    };

    commandHandlers["get_tracks"] = [this](const juce::var&) {
        auto json = MusiCode::TrackManager::getTracksInfo(audioEngine.getEdit());
        if (eventCallback != nullptr)
             eventCallback("tracks_list", json);
    };

    commandHandlers["get_clip_notes"] = [this](const juce::var& params) {
        juce::String clipIDStr = params.getProperty("clipID", "").toString();
        auto clipID = tracktion_engine::EditItemID::fromString(clipIDStr);
        
        juce::Array<juce::var> noteList;
        bool found = false;
        
        for (auto track : tracktion_engine::getAudioTracks(audioEngine.getEdit()))
        {
            for (auto clip : track->getClips())
            {
                if (clip->itemID == clipID)
                {
                    if (auto midiClip = dynamic_cast<tracktion_engine::MidiClip*>(clip))
                    {
                        auto& sequence = midiClip->getSequence();
                        for (auto* note : sequence.getNotes())
                        {
                            juce::DynamicObject::Ptr noteObj = new juce::DynamicObject();
                            noteObj->setProperty("pitch", note->getNoteNumber());
                            noteObj->setProperty("start", note->getStartBeat().inBeats());
                            noteObj->setProperty("length", note->getLengthBeats().inBeats());
                            noteObj->setProperty("velocity", (int)note->getVelocity());
                            noteList.add(noteObj.get());
                        }
                    }
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        
        if (eventCallback != nullptr)
        {
            juce::DynamicObject::Ptr result = new juce::DynamicObject();
            result->setProperty("clipID", clipIDStr);
            result->setProperty("notes", noteList);
            eventCallback("clip_notes_list", result.get());
        }
    };

    commandHandlers["set_track_input"] = [this](const juce::var& params) {
        tracktion_engine::EditItemID trackID;
        
        if (params.hasProperty("trackID"))
            trackID = MusiCode::TrackManager::stringToID(params.getProperty("trackID", "").toString());
        else
            trackID = MusiCode::TrackManager::getIDFromIndex(audioEngine.getEdit(), params.getProperty("trackIndex", 0));

        juce::String deviceName = params.getProperty("deviceName", "All MIDI Ins").toString();
        bool success = audioEngine.getMidiController().setTrackInput(trackID, deviceName);
        
        if (success)
            updateStatus("Track " + trackID.toString() + " Input -> " + deviceName, juce::Colours::green);
        else
            updateStatus("Failed to set Track " + trackID.toString() + " Input", juce::Colours::red);
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
