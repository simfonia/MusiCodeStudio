#include "PluginController.h"
#include "ParameterDispatcher.h"
#include "TrackManager.h"

PluginController::PluginController(te::Edit& e, te::Engine& eng) 
    : edit(e), engine(eng)
{
}

PluginController::~PluginController()
{
    activeWindow = nullptr;
}

void PluginController::showPluginWindow(te::EditItemID trackID)
{
    // 必須在 Message Thread 執行以操作 UI
    juce::MessageManager::callAsync([this, trackID]() {
        auto track = MusiCode::TrackManager::findAudioTrackByID(edit, trackID);
        if (track == nullptr)
        {
            DBG("PluginController: Track NOT FOUND for ID: " + trackID.toString());
            return;
        }

        // 獲取該軌道上的第一個插件
        if (auto p = track->pluginList[0])
        {
            // 使用 Tracktion 內建的 WindowState 系統
            if (p->windowState != nullptr)
            {
                p->windowState->showWindowExplicitly();
                DBG("PluginController: Showing window for plugin: " + p->getName());
            }
            else
            {
                DBG("PluginController: Plugin has no windowState: " + p->getName());
            }
        }
        else
        {
            DBG("PluginController: No plugins found on track: " + track->getName());
        }
    });
}

void PluginController::setPluginParameter(juce::String pluginNameMatch, juce::String paramID, float newValue)
{
    // 1. 尋找目標插件 (使用更強大的匹配邏輯)
    te::Plugin::Ptr targetPlugin = nullptr;

    for (auto audioTrack : te::getAudioTracks(edit))
    {
        for (auto p : audioTrack->pluginList)
        {
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

    // 2. 委託給 ParameterDispatcher
    if (targetPlugin != nullptr)
    {
        DBG("PluginController: Forwarding [" + paramID + "] to Dispatcher | Plugin: " + targetPlugin->getName());
        MusiCode::ParameterDispatcher::setParameter(targetPlugin, paramID, newValue);
    }
    else
    {
        DBG("PluginController: CANNOT FIND Plugin matching: " + pluginNameMatch);
    }
}
