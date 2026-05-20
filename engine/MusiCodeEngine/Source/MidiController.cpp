#include "MidiController.h"
#include <iostream>

namespace te = tracktion_engine;

MidiController::MidiController(te::Edit& e, te::Engine& eng)
    : edit(e), engine(eng)
{
    engine.getDeviceManager().deviceManager.addChangeListener(this);
    startTimerHz(30);
    refreshDevices();
}

MidiController::~MidiController()
{
    auto& dm = engine.getDeviceManager();
    for (auto& dev : juce::MidiInput::getAvailableDevices())
        dm.deviceManager.removeMidiInputDeviceCallback(dev.identifier, this);

    engine.getDeviceManager().deviceManager.removeChangeListener(this);
    stopTimer();
}

void MidiController::refreshDevices()
{
    auto& dm = engine.getDeviceManager();
    
    for (auto& dev : juce::MidiInput::getAvailableDevices())
    {
        dm.deviceManager.setMidiInputDeviceEnabled(dev.identifier, true);
        dm.deviceManager.addMidiInputDeviceCallback(dev.identifier, this);
    }

    dm.rescanMidiDeviceList();
    dm.dispatchPendingUpdates();
    
    for (auto midiIn : dm.getMidiInDevices())
    {
        midiIn->setMonitorMode(te::InputDevice::MonitorMode::on);
        midiIn->setEnabled(true);
    }
}

juce::var MidiController::getMidiInputsAsJson()
{
    juce::Array<juce::var> deviceList;
    auto& dm = engine.getDeviceManager();
    
    for (auto midiIn : dm.getMidiInDevices())
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("name", midiIn->getName());
        obj->setProperty("id", midiIn->getName());
        deviceList.add(obj.get());
    }
    
    return deviceList;
}

bool MidiController::setTrackInput(int trackIndex, const juce::String& deviceName)
{
    auto audioTracks = te::getAudioTracks(edit);
    if (trackIndex < 0 || trackIndex >= audioTracks.size())
        return false;

    auto track = audioTracks[trackIndex];
    edit.getTransport().ensureContextAllocated();

    bool success = false;
    for (auto instance : edit.getAllInputDevices())
    {
        if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice)
        {
            if (deviceName == "All MIDI Ins" || instance->getInputDevice().getName() == deviceName)
            {
                (void)instance->setTarget(track->itemID, true, &edit.getUndoManager(), 0);
                (void)instance->setRecordingEnabled(track->itemID, true);
                success = true;
            }
            else if (te::isOnTargetTrack(*instance, *track, 0))
            {
                (void)instance->setTarget(te::EditItemID(), false, nullptr, 0);
            }
        }
    }
    
    return success;
}

void MidiController::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &engine.getDeviceManager().deviceManager)
        refreshDevices();
}

void MidiController::timerCallback()
{
    auto audioTracks = te::getAudioTracks(edit);
    if (lastSignalLevels.size() != (size_t)audioTracks.size())
        lastSignalLevels.resize(audioTracks.size(), 0.0f);

    for (int i = 0; i < (int)audioTracks.size(); ++i)
    {
        lastSignalLevels[i] = juce::jmax(0.0f, lastSignalLevels[i] - 0.10f);
        if (lastSignalLevels[i] > 0.01f)
        {
            if (signalLevelCallback)
                signalLevelCallback(i, lastSignalLevels[i]);
        }
    }
}

void MidiController::handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message)
{
    if (message.isMidiClock()) return;
    if (!message.isNoteOn()) return;

    float level = message.getVelocity() / 127.0f;
    juce::String devName = source->getName();
    
    DBG("RAW MIDI In [" + devName + "]: Velocity " + juce::String((int)message.getVelocity()) + " -> Level " + juce::String(level, 2));

    auto audioTracks = te::getAudioTracks(edit);
    for (int i = 0; i < (int)audioTracks.size(); ++i)
    {
        bool isRouted = false;
        for (auto instance : edit.getAllInputDevices())
        {
            if (te::isOnTargetTrack(*instance, *audioTracks[i], 0))
            {
                if (instance->getInputDevice().getName() == devName || instance->getInputDevice().getName().contains("All MIDI"))
                {
                    isRouted = true;
                    break;
                }
            }
        }

        if (isRouted && level > lastSignalLevels[i])
        {
            lastSignalLevels[i] = level;
            if (signalLevelCallback)
                signalLevelCallback(i, level);
        }
    }
}
