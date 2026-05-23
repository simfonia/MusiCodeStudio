#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>
#include "../../engine/MusiCodeEngine/Source/TransportController.h"
#include "../../engine/MusiCodeEngine/Source/RecordingController.h"

namespace te = tracktion_engine;

class MainComponent  : public juce::Component,
                       private juce::Timer
{
public:
    MainComponent (te::Engine& e) : engine (e)
    {
        // 1. 初始化引擎設備
        engine.getDeviceManager().initialise (0, 2);
        
        addAndMakeVisible (logBox);
        logBox.setMultiLine (true);
        logBox.setReadOnly (true);

        log ("--- Recording Module Unit Test (State Machine Mode) ---");
        
        // 啟動狀態機
        testState = 0;
        startTimer (500); // 每 500ms 檢查一次狀態
        
        setSize (600, 400);
    }

    void log (const juce::String& message)
    {
        std::cout << message.toRawUTF8() << std::endl;
        juce::MessageManager::callAsync([this, message]{
            logBox.moveCaretToEnd();
            logBox.insertTextAtCaret (message + "\n");
        });
    }

    void timerCallback() override
    {
        switch (testState)
        {
            case 0: // Step 0: 建立虛擬設備
                log ("Step 0: Creating Virtual Device...");
                engine.getDeviceManager().createVirtualMidiDevice("Virtual Test Input");
                engine.getDeviceManager().rescanMidiDeviceList();
                testState = 1;
                break;

            case 1: // Step 1: 等待設備生效並建立 Edit
                log ("Step 1: Creating Edit...");
                {
                    auto editState = te::createEmptyEdit (engine);
                    te::Edit::Options options { engine };
                    options.editState = editState;
                    options.role = te::Edit::forEditing;
                    edit = std::make_unique<te::Edit> (options);
                    
                    transportCtrl = std::make_unique<MusiCode::TransportController>(*edit);
                    recordingCtrl = std::make_unique<MusiCode::RecordingController>(*edit, *transportCtrl);
                }
                testState = 2;
                break;

            case 2: // Step 2: 分配上下文與武裝
                log ("Step 2: Allocating Context & Arming...");
                edit->getTransport().ensureContextAllocated();
                
                {
                    auto track = te::getAudioTracks (*edit)[0];
                    if (track == nullptr)
                        track = dynamic_cast<te::AudioTrack*>(edit->insertNewTrack (te::TrackInsertPoint::getEndOfTracks (*edit), "Test Track", nullptr).get());
                    
                    bool armed = false;
                    auto instances = edit->getAllInputDevices();
                    log ("Found Input Instances: " + juce::String(instances.size()));

                    for (auto instance : instances)
                    {
                        if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice ||
                            instance->getInputDevice().getDeviceType() == te::InputDevice::virtualMidiDevice)
                        {
                            (void)instance->setTarget (track->itemID, true, &edit->getUndoManager(), 0);
                            instance->setRecordingEnabled (track->itemID, true);
                            armed = true;
                            log ("SUCCESS: Armed Device -> " + instance->getInputDevice().getName());
                        }
                    }

                    if (armed) {
                        testState = 3;
                    } else {
                        log ("Still no instances, retrying in next tick...");
                        // 保持在 State 2，等待 TE 建立 Instance
                    }
                }
                break;

            case 3: // Step 3: 開始錄音
                log ("Step 3: Starting Recording...");
                recordingCtrl->startRecording();
                testState = 4;
                frameCount = 0;
                break;

            case 4: // Step 4: 注入數據
                if (frameCount == 1) { // 錄音開始後一陣子注入
                    log ("Step 4: Injecting MIDI Data...");
                    auto msg = juce::MidiMessage::noteOn (1, 60, 0.8f);
                    engine.getDeviceManager().injectMIDIMessageToDefaultDevice(msg);
                } else if (frameCount == 3) {
                    auto msg = juce::MidiMessage::noteOff (1, 60);
                    engine.getDeviceManager().injectMIDIMessageToDefaultDevice(msg);
                    testState = 5;
                    frameCount = 0;
                }
                frameCount++;
                break;

            case 5: // Step 5: 停止錄音
                log ("Step 5: Stopping & Saving...");
                recordingCtrl->stopAndSave();
                testState = 6;
                break;

            case 6: // Step 6: 最終驗證
                {
                    auto track = te::getAudioTracks (*edit)[0];
                    auto clips = track->getClips();
                    log ("Final Verification:");
                    log ("Total Clips: " + juce::String (clips.size()));
                    
                    if (clips.size() > 0) {
                        log ("TEST PASSED: Recording generated a clip!");
                        juce::File("C:\\Workspace\\MusiCodeStudio\\tests\\RecordingModuleTestApp\\test_result.txt").replaceWithText("PASS");
                        juce::JUCEApplication::getInstance()->setApplicationReturnValue(0);
                    } else {
                        log ("TEST FAILED: No clip generated.");
                        juce::File("C:\\Workspace\\MusiCodeStudio\\tests\\RecordingModuleTestApp\\test_result.txt").replaceWithText("FAIL");
                        juce::JUCEApplication::getInstance()->setApplicationReturnValue(1);
                    }
                    
                    stopTimer();
                    juce::Timer::callAfterDelay(1000, [] {
                        juce::JUCEApplication::getInstance()->systemRequestedQuit();
                    });
                }
                break;
        }
    }

    void paint (juce::Graphics& g) override { g.fillAll (juce::Colours::black); }
    void resized() override { logBox.setBounds (getLocalBounds().reduced (10)); }

private:
    te::Engine& engine;
    std::unique_ptr<te::Edit> edit;
    std::unique_ptr<MusiCode::TransportController> transportCtrl;
    std::unique_ptr<MusiCode::RecordingController> recordingCtrl;
    juce::TextEditor logBox;
    int testState = 0;
    int frameCount = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

class RecordingTestApplication  : public juce::JUCEApplication
{
public:
    RecordingTestApplication() {}
    const juce::String getApplicationName() override       { return "RecordingModuleTest"; }
    const juce::String getApplicationVersion() override    { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override             { return true; }
    void initialise (const juce::String& commandLine) override
    {
        engine = std::make_unique<te::Engine> (getApplicationName());
        mainWindow.reset (new MainWindow (getApplicationName(), *engine));
    }
    void shutdown() override { mainWindow = nullptr; engine = nullptr; }
    void systemRequestedQuit() override { quit(); }

    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name, te::Engine& e)
            : DocumentWindow (name, juce::Colours::darkgrey, allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (e), true);
            setResizable (true, true);
            centreWithSize (600, 400);
            setVisible (true);
        }
        void closeButtonPressed() override { JUCEApplication::getInstance()->systemRequestedQuit(); }
    };

private:
    std::unique_ptr<te::Engine> engine;
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (RecordingTestApplication)
