#include "TransportController.h"

namespace MusiCode
{
    TransportController::TransportController(tracktion_engine::Edit& e)
        : edit(e), transport(e.getTransport())
    {
        // 初始同步：設置一個默認的 1 小節循環範圍 (4 拍 @ 120bpm = 2.0s)
        juce::MessageManager::callAsync([this]() {
            transport.setLoopRange({ tracktion::TimePosition::fromSeconds(0.0), tracktion::TimePosition::fromSeconds(2.0) });
            transport.looping = false;
        });
    }

    void TransportController::play(bool loop)
    {
        juce::MessageManager::callAsync([this, loop]() {
            transport.looping = loop;
            if (loop && !transport.isPlaying())
            {
                transport.setPosition(transport.getLoopRange().getStart());
            }
            transport.play(true);
        });
    }

    void TransportController::stop(bool returnToStart)
    {
        juce::MessageManager::callAsync([this, returnToStart]() {
            transport.stop(false, returnToStart);
        });
    }

    bool TransportController::isPlaying() const { return transport.isPlaying(); }

    void TransportController::setPosition(tracktion::TimePosition newPosition)
    {
        juce::MessageManager::callAsync([this, newPosition]() {
            transport.setPosition(newPosition);
            transport.ensureContextAllocated();
        });
    }

    tracktion::TimePosition TransportController::getPosition() const { return transport.getPosition(); }

    void TransportController::updateTempo()
    {
        // 安全檢查：確保 TempoSequence 有資料
        auto& tempos = edit.tempoSequence.getTempos();
        if (tempos.size() == 0) return;

        int denominator = 4;
        if (auto ts = edit.tempoSequence.getTimeSig(0))
            denominator = (int)ts->denominator;

        double effectiveBpm = userBpm * (4.0 / (double)denominator);
        
        tempos[0]->setBpm(effectiveBpm);
        edit.tempoSequence.updateTempoData();
        transport.editHasChanged();
    }

    void TransportController::setBpm(double newBpm)
    {
        juce::MessageManager::callAsync([this, newBpm]() {
            userBpm = newBpm;
            updateTempo();
        });
    }

    double TransportController::getBpm() const { return userBpm; }

    void TransportController::setTimeSignature(int numerator, int denominator)
    {
        juce::MessageManager::callAsync([this, numerator, denominator]() {
            if (auto ts = edit.tempoSequence.getTimeSig(0))
            {
                ts->numerator = numerator;
                ts->denominator = denominator;
                updateTempo();
            }
        });
    }

    std::pair<int, int> TransportController::getTimeSignature() const
    {
        if (auto ts = edit.tempoSequence.getTimeSig(0))
            return { (int)ts->numerator, (int)ts->denominator };
        return { 4, 4 };
    }

    void TransportController::setLoopRange(tracktion::TimeRange newRange)
    {
        juce::MessageManager::callAsync([this, newRange]() {
            transport.setLoopRange(newRange);
            transport.ensureContextAllocated();
        });
    }

    void TransportController::setLoopEnabled(bool shouldLoop)
    {
        juce::MessageManager::callAsync([this, shouldLoop]() {
            transport.looping = shouldLoop;
        });
    }

    bool TransportController::isLoopEnabled() const { return transport.looping; }

    void TransportController::setClickEnabled(bool enabled)
    {
        juce::MessageManager::callAsync([this, enabled]() {
            edit.clickTrackEnabled = enabled;
            if (enabled) {
                edit.clickTrackEmphasiseBars = true;
                edit.clickTrackGain = 1.0f;
            }
        });
    }

    bool TransportController::isClickEnabled() const { return edit.clickTrackEnabled; }
}
