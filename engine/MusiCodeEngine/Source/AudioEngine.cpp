#include "AudioEngine.h"

AudioEngine::AudioEngine()
    : engine ("MusiCodeEngine", std::make_unique<MusiCodeUIBehaviour>(), nullptr)
{
    using namespace tracktion_engine;

    // 1. 初始化音訊設備 (修正：請求 2 入 2 出以支援麥克風)
    engine.getDeviceManager().initialise(2, 2);

    // 2. 建立空的 Edit
    auto editState = createEmptyEdit(engine);
    auto id = ProjectItemID::fromProperty(editState, IDs::projectID);
    if (!id.isValid()) id = ProjectItemID::createNewID(0);

    Edit::Options options { engine };
    options.editState = editState;
    options.editProjectItemID = id;
    options.role = Edit::forEditing;
    options.numUndoLevelsToStore = 100;

    edit = std::make_unique<Edit>(options);

    // 3. 初始化插件控制器
    pluginController = std::make_unique<PluginController>(*edit, engine);

    // 4. 建立初始測試場景
    setupTestScene();
}

void AudioEngine::setupTestScene()
{
    using namespace tracktion_engine;
    
    auto track = edit->insertNewTrack(TrackInsertPoint::getEndOfTracks(*edit), IDs::TRACK, nullptr);
    if (auto audioTrack = dynamic_cast<AudioTrack*>(track.get()))
    {
        // 建立 4osc 並開啟濾波器
        auto newPlugin = edit->getPluginCache().createNewPlugin (FourOscPlugin::xmlTypeName, {});
        if (newPlugin != nullptr)
        {
            audioTrack->pluginList.insertPlugin (newPlugin, 0, nullptr);

            juce::ValueTree patch ("PLUGIN");
            patch.setProperty ("waveShape1", 3, nullptr); 
            patch.setProperty ("filterType", 1, nullptr);      // LP
            patch.setProperty ("filterSlope", 12, nullptr);
            patch.setProperty ("filterFreq", 69.0f, nullptr); 
            patch.setProperty ("filterResonance", 50.0f, nullptr);
            patch.setProperty ("filterAmount", 0.0f, nullptr);
            newPlugin->restorePluginStateFromValueTree(patch);
        }
        
        // 建立長音 MIDI Clip
        auto clip = audioTrack->insertMIDIClip({ tracktion::TimePosition::fromSeconds(0.0), tracktion::TimePosition::fromSeconds(60.0) }, nullptr);
        if (auto midiClip = dynamic_cast<MidiClip*>(clip.get()))
        {
            midiClip->getSequence().addNote(60, tracktion::BeatPosition::fromBeats(0.0), tracktion::BeatDuration::fromBeats(100.0), 100, 0, nullptr);
        }
    }
}

AudioEngine::~AudioEngine()
{
    edit = nullptr;
}

void AudioEngine::play()
{
    // Tracktion Engine 要求 Transport 操作必須在 Message Thread
    juce::MessageManager::callAsync([this]() {
        if (edit != nullptr)
        {
            auto& transport = edit->getTransport();
            // 如果已經在最後，先回到起點 (將 TimeDuration 轉為 TimePosition 進行比較)
            if (transport.getPosition() >= tracktion::toPosition(edit->getLength()))
                transport.setPosition(tracktion::TimePosition::fromSeconds(0.0));
            
            transport.play(false);
        }
    });
}

void AudioEngine::stop()
{
    juce::MessageManager::callAsync([this]() {
        if (edit != nullptr)
        {
            auto& transport = edit->getTransport();
            transport.stop(false, false);
            transport.setPosition(tracktion::TimePosition::fromSeconds(0.0));
        }
    });
}

bool AudioEngine::isPlaying() const
{
    return edit != nullptr && edit->getTransport().isPlaying();
}

void AudioEngine::setBpm(double newBpm)
{
    if (edit != nullptr)
    {
        // Tracktion Engine 的 BPM 設置通常透過 TempoSequence
        auto& tempoSequence = edit->tempoSequence;
        tempoSequence.getTempoAt(tracktion::TimePosition()).setBpm(newBpm);
    }
}

double AudioEngine::getBpm() const
{
    if (edit != nullptr)
        return edit->tempoSequence.getTempoAt(tracktion::TimePosition()).getBpm();
    
    return 120.0;
}

void AudioEngine::setPluginParameter(juce::String pluginName, juce::String paramID, float newValue)
{
    if (pluginController != nullptr)
        pluginController->setPluginParameter(pluginName, paramID, newValue);
}


