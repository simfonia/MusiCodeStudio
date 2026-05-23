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
                // 依照研究結果：stop(false, false) 代表 discardRecordings = false
                transport.stop(false, false);
            }
        });
    }

    void RecordingController::stopAndDiscard()
    {
        juce::MessageManager::callAsync([this]() {
            auto& transport = transportController.getTransport();
            if (transport.isRecording())
            {
                // stop(true, false) 代表捨棄錄音
                transport.stop(true, false);
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
