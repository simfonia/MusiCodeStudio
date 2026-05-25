#include "RecordingController.h"

namespace MusiCode
{
    RecordingController::RecordingController(tracktion_engine::Edit& e, TransportController& tc)
        : edit(e), transportController(tc)
    {
    }

    void RecordingController::startRecording()
    {
        juce::MessageManager::callAsync([this]() {
            auto& transport = transportController.getTransport();
            
            auto loopRange = transport.getLoopRange();
            auto targetRecStart = loopRange.getStart();
            bool isUserLooping = transportController.isLoopEnabled();

            // 1. 紀錄錄製前的節拍器狀態，以便後續恢復
            wasClickEnabledBeforeRecording = edit.clickTrackEnabled;

            // 2. 設定引擎內建倒數模式 (1 小節)
            edit.setCountInMode(tracktion_engine::Edit::CountIn::oneBar);

            // 3. 模式對齊
            if (isUserLooping)
            {
                edit.recordingPunchInOut = false;
                transport.looping = true;
            }
            else
            {
                edit.recordingPunchInOut = true;
                transport.looping = false;
            }

            // 4. 定位到錄音目標起點
            transport.setPosition(targetRecStart);

            // 5. [關鍵修正] 暫時開啟節拍器，確保倒數期間聽得到聲音
            // 即便使用者平常沒開節拍器，倒數時通常也需要 Click
            edit.clickTrackEnabled = true;

            // 6. 啟動錄製
            if (!transport.isRecording())
            {
                transport.record(false);
            }
            
            DBG("RecordingController: Native Count-in (1 Bar) engaged. Start position: " + 
                juce::String(targetRecStart.inSeconds(), 2) + "s");
        });
    }

    void RecordingController::stopAndSave()
    {
        if (isProcessing) return;
        isProcessing = true;

        juce::MessageManager::callAsync([this]() {
            auto& transport = transportController.getTransport();
            if (transport.isRecording())
            {
                transport.stop(false, false);
                
                // 7. [關鍵修正] 恢復錄製前的節拍器狀態
                edit.clickTrackEnabled = wasClickEnabledBeforeRecording;
                
                // 重置錄音專用狀態
                edit.recordingPunchInOut = false;
                edit.setCountInMode(tracktion_engine::Edit::CountIn::none);
                
                transport.ensureContextAllocated();
                transport.stop(false, false);
                
                DBG("RecordingController: Recording Stopped & Saved. Metronome restored.");
            }
            isProcessing = false;
        });
    }

    void RecordingController::stopAndDiscard()
    {
        juce::MessageManager::callAsync([this]() {
            auto& transport = transportController.getTransport();
            if (transport.isRecording())
            {
                transport.stop(true, false);
                
                // 恢復錄製前的節拍器狀態
                edit.clickTrackEnabled = wasClickEnabledBeforeRecording;

                edit.recordingPunchInOut = false;
                edit.setCountInMode(tracktion_engine::Edit::CountIn::none);
                transport.stop(false, true);
                DBG("RecordingController: Recording Discarded. Metronome restored.");
            }
        });
    }

    bool RecordingController::isRecording() const
    {
        return transportController.getTransport().isRecording();
    }

    bool RecordingController::isArmed(tracktion_engine::EditItemID trackID) const
    {
        for (auto instance : edit.getAllInputDevices())
        {
            if (instance->isRecordingEnabled(trackID))
                return true;
        }
        return false;
    }
}
