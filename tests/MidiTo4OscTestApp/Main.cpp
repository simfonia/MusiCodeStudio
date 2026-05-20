#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion_engine;

//==============================================================================
class MainComponent  : public juce::Component,
                       private juce::Timer,
                       private juce::MidiInputCallback
{
public:
    MainComponent (te::Engine& e) : engine (e)
    {
        engine.getDeviceManager().deviceManager.initialise (0, 2, nullptr, true);
        
        auto editState = te::createEmptyEdit (engine);
        te::Edit::Options options { engine };
        options.editState = editState;
        options.role = te::Edit::forEditing;
        edit = std::make_unique<te::Edit> (options);
        
        addAndMakeVisible (logBox);
        logBox.setMultiLine (true);
        logBox.setReadOnly (true);
        logBox.setReturnKeyStartsNewLine (true);

        log ("--- Starting MIDI -> 4OSC Diagnostic (V3 - Order Fix) ---");
        startTime = juce::Time::getMillisecondCounter();
        startTimer (100); 
        
        setSize (600, 400);
    }

    ~MainComponent() override
    {
        for (auto& in : diagnosticInputs)
            in->stop();
        diagnosticInputs.clear();
    }

    void log (const juce::String& message)
    {
        logBox.moveCaretToEnd();
        logBox.insertTextAtCaret (message + "\n");
    }

    void handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message) override
    {
        if (message.isMidiClock()) return;
        juce::MessageManager::callAsync ([this, msg = message, name = source->getName()] {
            log ("MIDI RAW [" + name + "]: " + msg.getDescription());
        });
    }

    void timerCallback() override
    {
        auto& dm = engine.getDeviceManager();
        auto midiInputs = dm.getMidiInDevices();

        if (midiInputs.size() == 0 && (juce::Time::getMillisecondCounter() - startTime < 3000))
        {
            auto allMidiDevices = juce::MidiInput::getAvailableDevices();
            for (auto& dev : allMidiDevices)
                dm.deviceManager.setMidiInputDeviceEnabled (dev.identifier, true);
            
            dm.rescanMidiDeviceList();
            return;
        }

        stopTimer();
        setupLogic();
    }

    void setupLogic()
    {
        auto& dm = engine.getDeviceManager();
        
        // 1. 強制 Dump 訊號 (硬體層)
        auto allMidiDevices = juce::MidiInput::getAvailableDevices();
        for (auto& dev : allMidiDevices)
        {
            if (auto in = juce::MidiInput::openDevice (dev.identifier, this))
            {
                in->start();
                diagnosticInputs.push_back (std::move (in));
            }
        }

        // 2. 獲取 TE 設備並開啟監聽模式
        auto midiInputs = dm.getMidiInDevices();
        log ("TE-wrapped MIDI Inputs: " + juce::String (midiInputs.size()));

        for (auto& midiIn : midiInputs)
        {
            midiIn->setMonitorMode (te::InputDevice::MonitorMode::automatic);
            midiIn->setEnabled (true);
        }

        // 3. 確保軌道與插件就緒
        auto track = te::getAudioTracks (*edit)[0];
        if (track != nullptr)
        {
            for (auto p : track->pluginList.getPlugins())
                p->removeFromParent();

            if (auto p = edit->getPluginCache().createNewPlugin (te::FourOscPlugin::xmlTypeName, {}))
            {
                track->pluginList.insertPlugin (p, 0, nullptr);
                log ("4OSC Plugin ready on Track 0.");
                
                if (auto filterParam = p->getAutomatableParameterByID ("filterType"))
                    filterParam->setParameter (1.0f, juce::sendNotification);
            }
        }

        // --- 核心修正：必須先呼叫 ensureContextAllocated，InputDeviceInstance 才會被建立 ---
        log ("Allocating Playback Context...");
        edit->getTransport().ensureContextAllocated();
        
        // 4. 此時才能獲取實例並路由
        int routedCount = 0;
        auto instances = edit->getAllInputDevices();
        log ("Available Input Instances: " + juce::String (instances.size()));

        for (auto instance : instances)
        {
            if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice)
            {
                log ("Routing Device Instance: " + instance->getInputDevice().getName());
                
                // 執行路由
                auto res = instance->setTarget (track->itemID, true, &edit->getUndoManager(), 0);
                juce::ignoreUnused (res);
                
                // 強制開啟錄音/監聽開關
                instance->setRecordingEnabled (track->itemID, true);
                
                if (instance->isLivePlayEnabled (*track))
                {
                    log ("SUCCESS: Live Monitoring ACTIVE for " + instance->getInputDevice().getName());
                    routedCount++;
                }
                else
                {
                    log ("FAIL: Live Monitoring INACTIVE for " + instance->getInputDevice().getName());
                }
            }
        }
        log ("Total active routed instances: " + juce::String (routedCount));

        edit->getTransport().play (false);
        log ("Transport: Playing");
        
        if (auto device = dm.deviceManager.getCurrentAudioDevice())
            log ("Audio Output: " + device->getName());
            
        log ("--- Setup Complete. Try playing again! ---");
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        logBox.setBounds (getLocalBounds().reduced (10));
    }

private:
    te::Engine& engine;
    std::unique_ptr<te::Edit> edit;
    juce::TextEditor logBox;
    juce::uint32 startTime;
    std::vector<std::unique_ptr<juce::MidiInput>> diagnosticInputs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

//==============================================================================
class MidiTo4OscApplication  : public juce::JUCEApplication
{
public:
    MidiTo4OscApplication() {}

    const juce::String getApplicationName() override       { return "MidiTo4OscTest"; }
    const juce::String getApplicationVersion() override    { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise (const juce::String& commandLine) override
    {
        engine = std::make_unique<te::Engine> (getApplicationName());
        mainWindow.reset (new MainWindow (getApplicationName(), *engine));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
        engine = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override {}

    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name, te::Engine& e)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (e), true);
            setResizable (true, true);
            centreWithSize (600, 400);
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<te::Engine> engine;
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (MidiTo4OscApplication)
