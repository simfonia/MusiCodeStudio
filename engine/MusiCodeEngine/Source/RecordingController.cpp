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
            if (!transport.isRecording())
            {
                // 依照手冊 [3. 播放與傳輸]：呼叫 record(false) 啟動錄製
                transport.record(false);
            }
        });
    }

    void RecordingController::stopAndSave()
    {
        juce::MessageManager::callAsync([this]() {
            auto& transport = transportController.getTransport();
            if (transport.isRecording())
            {
                // 1. 停止錄製
                transport.stop(false, false);
                
                // 2. [關鍵] 強制引擎重新掃描內容，確保新 Clip 立即生效
                transport.ensureContextAllocated();
                
                // 3. 徹底重置狀態機
                transport.stop(false, true);

                // 4. 自動將播放頭移回 0，方便立即試聽
                transport.setPosition(tracktion::TimePosition::fromSeconds(0.0));
                
                DBG("RecordingController: Context Rebuilt and Playhead Resetted.");
            }
        });
    }

    void RecordingController::stopAndDiscard()
    {
        juce::MessageManager::callAsync([this]() {
            auto& transport = transportController.getTransport();
            if (transport.isRecording())
            {
                transport.stop(true, false);
                transport.stop(false, true); // 徹底重置
                DBG("RecordingController: Recording Discarded. Transport state reset.");
            }
        });
    }

    bool RecordingController::isRecording() const
    {
        return transportController.getTransport().isRecording();
    }

    bool RecordingController::isArmed(tracktion_engine::EditItemID trackID) const
    {
        // 檢查該軌道是否有任何 InputDeviceInstance 開啟了 RecordingEnabled
        for (auto instance : edit.getAllInputDevices())
        {
            if (instance->isRecordingEnabled(trackID))
                return true;
        }
        return false;
    }
}
