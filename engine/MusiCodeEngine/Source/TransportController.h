#pragma once
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

namespace MusiCode
{
    /**
     * TransportController - 負責控制播放、停止、定位與 BPM。
     * 封裝了 tracktion_engine::TransportControl。
     */
    class TransportController
    {
    public:
        TransportController(tracktion_engine::Edit& edit);
        ~TransportController() = default;

        // 播放控制
        void play(bool loop = false);
        void stop(bool returnToStart = true);
        bool isPlaying() const;
        
        // 定位控制
        void setPosition(tracktion::TimePosition newPosition);
        tracktion::TimePosition getPosition() const;

        // BPM 與 拍號 控制
        void setBpm(double newBpm);
        double getBpm() const;
        void setTimeSignature(int numerator, int denominator);
        std::pair<int, int> getTimeSignature() const;

        // 循環 (Looping)
        void setLoopRange(tracktion::TimeRange newRange);
        void setLoopEnabled(bool shouldLoop);
        bool isLoopEnabled() const;

        // 節拍器 (Metronome)
        void setClickEnabled(bool enabled);
        bool isClickEnabled() const;

        // 獲取底層引用
        tracktion_engine::TransportControl& getTransport() { return transport; }

    private:
        void updateTempo();
        
        tracktion_engine::Edit& edit;
        tracktion_engine::TransportControl& transport;
        double userBpm = 120.0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportController)
    };
}
