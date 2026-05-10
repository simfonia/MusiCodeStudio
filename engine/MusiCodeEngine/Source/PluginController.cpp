#include "PluginController.h"

PluginController::PluginController(te::Edit& e, te::Engine& eng) 
    : edit(e), engine(eng)
{
}

PluginController::~PluginController()
{
    activeWindow = nullptr;
}

void PluginController::showPluginWindow(int trackIndex)
{
    // ... existing implementation ...
}

void PluginController::setPluginParameter(juce::String pluginNameMatch, juce::String paramID, float newValue)
{
    // 必須在 Message Thread 執行以符合 Tracktion 架構
    juce::MessageManager::callAsync([this, pluginNameMatch, paramID, newValue]() {
        te::Plugin::Ptr targetPlugin = nullptr;

        // 全域搜尋匹配名稱的插件
        for (auto audioTrack : te::getAudioTracks(edit))
        {
            for (auto p : audioTrack->pluginList)
            {
                if (p->getName().containsIgnoreCase(pluginNameMatch))
                {
                    targetPlugin = p;
                    break;
                }
            }
            if (targetPlugin != nullptr) break;
        }

        if (targetPlugin != nullptr)
        {
            if (auto param = targetPlugin->getAutomatableParameterByID(paramID))
            {
                float rawValue = param->valueRange.convertFrom0to1(newValue);
                
                // 直接更新 ValueTree State (最高優先級，DSP 同步最穩定)
                targetPlugin->state.setProperty(paramID, rawValue, nullptr);

                // 同步更新參數物件（不使用 Gesture 避免併發 Assertion）
                param->setParameter(rawValue, juce::sendNotification);
            }
        }
    });
}
