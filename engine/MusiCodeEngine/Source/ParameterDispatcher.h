#pragma once
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

namespace MusiCode
{
    /**
     * ParameterDispatcher - 負責處理插件參數的標準化對接與同步。
     * 封裝了 "Dual Update Pattern" (同時更新 AutomatableParameter 與 ValueTree)。
     */
    class ParameterDispatcher
    {
    public:
        /** 
         * 設定插件參數
         * @param plugin 目標插件
         * @param paramID 參數識別碼 (如 "filterFreq")
         * @param normalizedValue 標準化數值 (0.0 ~ 1.0)
         */
        static void setParameter(tracktion_engine::Plugin::Ptr plugin, 
                                 const juce::String& paramID, 
                                 float normalizedValue)
        {
            if (plugin == nullptr) return;

            // 確保在 Message Thread 執行以符合 Tracktion 架構
            juce::MessageManager::callAsync([plugin, paramID, normalizedValue]() {
                if (auto param = plugin->getAutomatableParameterByID(paramID))
                {
                    // 1. 執行範圍轉換 (0~1 -> Raw)
                    float rawValue = param->valueRange.convertFrom0to1(normalizedValue);

                    // 2. 更新參數物件 (控制實例與處理鏈)
                    param->setParameter(rawValue, juce::sendNotification);

                    // 3. 更新 ValueTree (持久化與狀態同步)
                    plugin->state.setProperty(paramID, rawValue, nullptr);

                    DBG("ParamDispatcher [DualUpdate]: " + paramID + " -> " + juce::String(rawValue, 2));
                }
                else
                {
                    // Fallback: 找不到參數物件時，嘗試直接寫入 ValueTree (適用於隱藏屬性或 CachedValue)
                    // 對於離散參數 (如 filterType)，我們需要將 0~1 映射到合理的整數範圍
                    // 這裡針對 4OSC 的常見離散屬性進行簡易對接
                    float mappedValue = normalizedValue;
                    
                    if (paramID == "filterType") mappedValue = std::round(normalizedValue * 4.0f); // 0~4
                    else if (paramID.contains("waveShape")) mappedValue = std::round(normalizedValue * 3.0f); // 0~3

                    plugin->state.setProperty(paramID, mappedValue, nullptr);
                    DBG("ParamDispatcher [Fallback]: " + paramID + " (Mapped: " + juce::String(mappedValue, 1) + ")");
                }
            });
        }

        /** 
         * 批量設定參數 (初始化測試場景用)
         */
        static void setParameters(tracktion_engine::Plugin::Ptr plugin, 
                                  const std::map<juce::String, float>& params)
        {
            for (auto const& [id, val] : params)
                setParameter(plugin, id, val);
        }
    };
}
