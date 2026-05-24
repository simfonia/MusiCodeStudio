#include "MidiController.h"
#include "TrackManager.h"
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
        // 預設關閉全域監聽，由 setTrackInput 決定
        midiIn->setMonitorMode(te::InputDevice::MonitorMode::off);
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

bool MidiController::setTrackInput(te::EditItemID trackID, const juce::String& deviceName)
{
    auto track = MusiCode::TrackManager::findAudioTrackByID(edit, trackID);
    if (track == nullptr) return false;

    bool success = false;
    for (auto instance : edit.getAllInputDevices())
    {
        if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice ||
            instance->getInputDevice().getDeviceType() == te::InputDevice::virtualMidiDevice)
        {
            if (deviceName == "All MIDI Ins" || instance->getInputDevice().getName() == deviceName)
            {
                // 設定目標軌道，並開啟武裝
                (void)instance->setTarget(track->itemID, true, &edit.getUndoManager(), 0);
                (void)instance->setRecordingEnabled(track->itemID, true);
                
                // 開啟監控，確保彈奏時有聲音
                instance->getInputDevice().setMonitorMode(te::InputDevice::MonitorMode::on);
                success = true;
            }
            else if (te::isOnTargetTrack(*instance, *track, 0))
            {
                // 若原本是指向這軌但現在要更換，則取消舊的武裝
                (void)instance->setTarget(te::EditItemID(), false, nullptr, 0);
            }
        }
    }
    
    // 關鍵：在所有路由變更後重新分配上下文，使錄音武裝生效
    edit.getTransport().ensureContextAllocated();
    
    return success;
}

void MidiController::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &engine.getDeviceManager().deviceManager)
        refreshDevices();
}

void MidiController::timerCallback()
{
    // 平滑衰減所有軌道的訊號強度
    for (auto& pair : lastSignalLevels)
    {
        pair.second = juce::jmax(0.0f, pair.second - 0.10f);
        if (pair.second > 0.01f)
        {
            if (signalLevelCallback)
                signalLevelCallback(pair.first, pair.second);
        }
    }
}

void MidiController::handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message)
{
    if (message.isMidiClock()) return;
    if (!message.isNoteOn()) return;

    float level = message.getVelocity() / 127.0f;
    juce::String devName = source->getName();
    
    auto audioTracks = te::getAudioTracks(edit);
    for (auto track : audioTracks)
    {
        bool isRouted = false;
        for (auto instance : edit.getAllInputDevices())
        {
            if (te::isOnTargetTrack(*instance, *track, 0))
            {
                if (instance->getInputDevice().getName() == devName || instance->getInputDevice().getName().contains("All MIDI"))
                {
                    isRouted = true;
                    break;
                }
            }
        }

        if (isRouted && level > lastSignalLevels[track->itemID])
        {
            lastSignalLevels[track->itemID] = level;
            if (signalLevelCallback)
                signalLevelCallback(track->itemID, level);
        }
    }
}
