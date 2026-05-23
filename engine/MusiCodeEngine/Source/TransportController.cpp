#include "TransportController.h"

namespace MusiCode
{
    TransportController::TransportController(tracktion_engine::Edit& e)
        : edit(e), transport(e.getTransport())
    {
    }

    void TransportController::play(bool loop)
    {
        juce::MessageManager::callAsync([this, loop]() {
            // 如果當前在結尾，先回到起點
            if (transport.getPosition() >= tracktion::toPosition(edit.getLength()))
                transport.setPosition(tracktion::TimePosition::fromSeconds(0.0));
            
            transport.looping = loop;
            transport.play(false);
        });
    }

    void TransportController::stop(bool returnToStart)
    {
        juce::MessageManager::callAsync([this, returnToStart]() {
            transport.stop(false, false);
            if (returnToStart)
                transport.setPosition(tracktion::TimePosition::fromSeconds(0.0));
        });
    }

    bool TransportController::isPlaying() const
    {
        return transport.isPlaying();
    }

    void TransportController::setPosition(tracktion::TimePosition newPosition)
    {
        juce::MessageManager::callAsync([this, newPosition]() {
            transport.setPosition(newPosition);
        });
    }

    tracktion::TimePosition TransportController::getPosition() const
    {
        return transport.getPosition();
    }

    void TransportController::setBpm(double newBpm)
    {
        // Tempo 變更通常涉及 ValueTree，也應在 Message Thread
        juce::MessageManager::callAsync([this, newBpm]() {
            edit.tempoSequence.getTempoAt(tracktion::TimePosition()).setBpm(newBpm);
        });
    }

    double TransportController::getBpm() const
    {
        return edit.tempoSequence.getTempoAt(tracktion::TimePosition()).getBpm();
    }
}
