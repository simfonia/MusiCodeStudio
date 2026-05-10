#include "AudioEngine.h"

AudioEngine::AudioEngine()
    : engine("MusiCodeEngine")
{
    using namespace tracktion_engine;

    // 1. 初始化音訊設備 (開啟預設輸出)
    engine.getDeviceManager().initialise(0, 2);

    // 2. 建立空的 Edit 狀態
    auto editState = createEmptyEdit(engine);
    
    // 獲取或建立一個有效的 ProjectItemID
    auto id = ProjectItemID::fromProperty(editState, IDs::projectID);
    if (!id.isValid())
        id = ProjectItemID::createNewID(0);

    // 3. 使用 Options 初始化 Edit
    Edit::Options options { engine };
    options.editState = editState;
    options.editProjectItemID = id;
    options.role = Edit::forEditing;
    options.numUndoLevelsToStore = 100;

    edit = std::make_unique<Edit>(options);

    // 4. 加入測試音軌與插件
    auto track = edit->insertNewTrack(TrackInsertPoint::getEndOfTracks(*edit), IDs::TRACK, nullptr);
    if (auto audioTrack = dynamic_cast<AudioTrack*>(track.get()))
    {
        // 建立 4osc 插件 (使用官方常數避免字串錯誤)
        auto newPlugin = edit->getPluginCache().createNewPlugin (FourOscPlugin::xmlTypeName, {});
        if (newPlugin != nullptr)
        {
            audioTrack->pluginList.insertPlugin (newPlugin, 0, nullptr);
        }
        
        // 加入一個 MIDI Clip 並放一個長音 (C4, Note 60)
        auto clip = audioTrack->insertMIDIClip({ tracktion::TimePosition::fromSeconds(0.0), tracktion::TimePosition::fromSeconds(10.0) }, nullptr);
        if (auto midiClip = dynamic_cast<MidiClip*>(clip.get()))
        {
            midiClip->getSequence().addNote(60, tracktion::BeatPosition::fromBeats(0.0), tracktion::BeatDuration::fromBeats(4.0), 100, 0, nullptr);
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
            edit->getTransport().play(false);
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
