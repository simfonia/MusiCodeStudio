#pragma once
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>
#include "TransportController.h"

namespace MusiCode
{
    /**
     * RecordingController - 負責錄音狀態管理、武裝檢查與 Clip 生成。
     */
    class RecordingController
    {
    public:
        RecordingController(tracktion_engine::Edit& edit, TransportController& transportController);
        ~RecordingController() = default;

        // 錄音控制
        void startRecording();
        void stopAndSave();
        void stopAndDiscard();
        
        // 狀態查詢
        bool isRecording() const;
        bool isArmed(tracktion_engine::EditItemID trackID) const;

    private:
        tracktion_engine::Edit& edit;
        TransportController& transportController;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordingController)
    };
}
