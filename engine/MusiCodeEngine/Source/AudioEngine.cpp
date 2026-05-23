#include "AudioEngine.h"
#include "ParameterDispatcher.h"
#include <iostream>

namespace te = tracktion_engine;

AudioEngine::AudioEngine()
    : engine ("MusiCodeEngine", std::make_unique<MusiCodeUIBehaviour>(), nullptr)
{
    using namespace tracktion_engine;

    engine.getDeviceManager().initialise(2, 2);

    auto editState = createEmptyEdit(engine);
    auto id = ProjectItemID::fromProperty(editState, IDs::projectID);
    if (!id.isValid()) id = ProjectItemID::createNewID(0);

    Edit::Options options { engine };
    options.editState = editState;
    options.editProjectItemID = id;
    options.role = Edit::forEditing;
    options.numUndoLevelsToStore = 100;

    edit = std::make_unique<Edit>(options);

    pluginController = std::make_unique<PluginController>(*edit, engine);
    midiController = std::make_unique<MidiController>(*edit, engine);
    
    // 初始化新控制器
    transportController = std::make_unique<MusiCode::TransportController>(*edit);
    recordingController = std::make_unique<MusiCode::RecordingController>(*edit, *transportController);

    setupTestScene();
}

void AudioEngine::setupTestScene()
{
    using namespace tracktion_engine;
    DBG("setupTestScene: Starting...");
    
    transportController->getTransport().ensureContextAllocated();

    // --- 修正：優先使用現有軌道，避免 Index 偏移 ---
    auto audioTracks = getAudioTracks(*edit);
    AudioTrack* audioTrack = nullptr;

    if (audioTracks.size() > 0)
    {
        audioTrack = audioTracks[0];
        DBG("setupTestScene: Reusing existing Track 0 | ID: " + audioTrack->itemID.toString());
    }
    else
    {
        auto track = edit->insertNewTrack(TrackInsertPoint::getEndOfTracks(*edit), "TRACK", nullptr);
        audioTrack = dynamic_cast<AudioTrack*>(track.get());
        if (audioTrack) DBG("setupTestScene: Created new Track 0 | ID: " + audioTrack->itemID.toString());
    }

    if (audioTrack != nullptr)
    {
        audioTrack->setName("MusiCode Main Track");

        // 確保 4OSC 真的被加入
        while (auto p = audioTrack->pluginList.findFirstPluginOfType<FourOscPlugin>())
            p->removeFromParent();

        auto newPlugin = edit->getPluginCache().createNewPlugin (FourOscPlugin::xmlTypeName, {});
        if (newPlugin != nullptr)
        {
            audioTrack->pluginList.insertPlugin (newPlugin, 0, nullptr);
            newPlugin->setEnabled(true);
            
            // 使用 ParameterDispatcher 進行統一初始化
            MusiCode::ParameterDispatcher::setParameters(newPlugin, {
                {"waveShape1", 1.0f},      // 1.0 * 3 = 3 (Sawtooth)
                {"filterType", 0.25f},     // 0.25 * 4 = 1 (Low Pass)
                {"osc1Gain", 1.0f},
                {"filterFreq", 0.8f},
                {"filterResonance", 0.5f},
                {"gain", 0.8f}
            });

            DBG("setupTestScene: 4OSC Initialized via ParameterDispatcher");
        }

        // --- 補回 MIDI Clip 建立 (解決 Play 無聲) ---
        auto clip = audioTrack->insertMIDIClip({ tracktion::TimePosition::fromSeconds(0.0), tracktion::TimePosition::fromSeconds(60.0) }, nullptr);
        if (auto midiClip = dynamic_cast<MidiClip*>(clip.get()))
        {
            // 加入一連串長音
            midiClip->getSequence().addNote(60, tracktion::BeatPosition::fromBeats(0.0), tracktion::BeatDuration::fromBeats(4.0), 100, 0, nullptr);
            midiClip->getSequence().addNote(64, tracktion::BeatPosition::fromBeats(4.0), tracktion::BeatDuration::fromBeats(4.0), 100, 0, nullptr);
            midiClip->getSequence().addNote(67, tracktion::BeatPosition::fromBeats(8.0), tracktion::BeatDuration::fromBeats(4.0), 100, 0, nullptr);
            DBG("setupTestScene: MIDI Test Clip Created");
        }
    }
    
    transportController->setPosition(tracktion::TimePosition::fromSeconds(0.0));
}

AudioEngine::~AudioEngine()
{
    edit = nullptr;
}

void AudioEngine::play()
{
    transportController->play();
}

void AudioEngine::stop()
{
    if (recordingController->isRecording())
        recordingController->stopAndSave();
    else
        transportController->stop();
}

void AudioEngine::record()
{
    recordingController->startRecording();
}

bool AudioEngine::isPlaying() const
{
    return transportController->isPlaying();
}

bool AudioEngine::isRecording() const
{
    return recordingController->isRecording();
}

void AudioEngine::setBpm(double newBpm)
{
    transportController->setBpm(newBpm);
}

double AudioEngine::getBpm() const
{
    return transportController->getBpm();
}

void AudioEngine::setPluginParameter(juce::String pluginName, juce::String paramID, float newValue)
{
    if (pluginController != nullptr)
        pluginController->setPluginParameter(pluginName, paramID, newValue);
}
