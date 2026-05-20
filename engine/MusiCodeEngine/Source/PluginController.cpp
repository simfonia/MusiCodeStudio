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
                // 同時檢查內部名稱與顯示名稱
                if (p->getName().containsIgnoreCase(pluginNameMatch) || 
                    p->getShortName(256).containsIgnoreCase(pluginNameMatch) ||
                    pluginNameMatch.containsIgnoreCase(p->getName()))
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
                // 執行範圍轉換：從 0.0~1.0 映射到插件原始範圍 (例如 Pitch: 0~135)
                float rawValue = param->valueRange.convertFrom0to1(newValue);

                juce::String trackInfo = "Unknown Track";
                if (auto* t = targetPlugin->getOwnerTrack())
                    trackInfo = t->getName() + " (ID: " + t->itemID.toString() + ")";

                DBG("DUAL UPDATE [" + paramID + "] -> RAW: " + juce::String(rawValue, 2) + " | Track: " + trackInfo);
                
                // 1. 更新參數物件 (處理鏈同步)
                param->setParameter(rawValue, juce::sendNotification);
                
                // 2. 更新 ValueTree 屬性 (持久化同步)
                targetPlugin->state.setProperty(paramID, rawValue, nullptr);
            }
            else
            {
                // 回退機制：如果找不到對應的參數物件，嘗試直接更新 ValueTree 屬性
                // 這適用於像 filterType 這種 CachedValue 屬性
                DBG("FALLBACK UPDATE [" + paramID + "] -> " + juce::String(newValue, 2));
                targetPlugin->state.setProperty(paramID, newValue, nullptr);
            }
        }
        else
        {
            DBG("Plugin NOT FOUND matching: " + pluginNameMatch);
        }
    });
}
