#pragma once
#include <JuceHeader.h>

namespace te = tracktion_engine;

/**
 * PluginWindow - 用於包裝插件原生介面的 JUCE 視窗
 */
class PluginWindow : public juce::DocumentWindow
{
public:
    PluginWindow(te::Plugin& plug)
        : DocumentWindow(plug.getName(), juce::Colours::lightgrey, closeButton, true),
          plugin(plug)
    {
        setUsingNativeTitleBar(true);
        
        // 嘗試建立插件編輯器
        editor = plugin.createEditor();
        
        if (editor != nullptr)
        {
            setContentNonOwned(editor.get(), true);
        }
        else
        {
            // 如果插件沒有原生介面 (例如 4OSC 在此版本中可能未開放)，顯示佔位提示
            auto* fallbackLabel = new juce::Label();
            fallbackLabel->setText("Plugin: " + plugin.getName() + "\n(No Native Editor Available)", juce::dontSendNotification);
            fallbackLabel->setJustificationType(juce::Justification::centred);
            fallbackLabel->setColour(juce::Label::textColourId, juce::Colours::black);
            setContentOwned(fallbackLabel, true);
        }
        
        setResizable(true, false);
        setCentreRelative(0.5f, 0.5f);
        setSize(400, 300);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        // 這裡僅隱藏視窗，不銷毀，因為是由 WindowState 管理
        setVisible(false);
    }

private:
    te::Plugin& plugin;
    std::unique_ptr<te::Plugin::EditorComponent> editor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginWindow)
};

/**
 * MusiCodeUIBehaviour - 繼承自 te::UIBehaviour，定義引擎如何處理 UI 事件
 */
class MusiCodeUIBehaviour : public te::UIBehaviour
{
public:
    MusiCodeUIBehaviour() = default;

    std::unique_ptr<juce::Component> createPluginWindow(te::PluginWindowState& pws) override
    {
        if (auto* ws = dynamic_cast<te::Plugin::WindowState*>(&pws))
            return std::make_unique<PluginWindow>(ws->plugin);

        return {}; 
    }
};

/**
 * PluginController - 模組化管理插件視窗的控制類別
 */
class PluginController
{
public:
    PluginController(te::Edit& edit, te::Engine& engine);
    ~PluginController();

    // 顯示指定軌道上第一個插件的視窗
    void showPluginWindow(int trackIndex);

    /** 在整個 Edit 中尋找名稱匹配的插件並設定參數 (具備執行緒安全性與全域搜尋) */
    void setPluginParameter(juce::String pluginNameMatch, juce::String paramID, float newValue);

private:
    te::Edit& edit;
    te::Engine& engine;
    std::unique_ptr<juce::Component> activeWindow; // 改用 Component 基類以符合 UIBehaviour 規範

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginController)
};
