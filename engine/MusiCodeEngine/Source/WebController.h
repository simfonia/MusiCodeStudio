#pragma once
#include <JuceHeader.h>
#include "CommandRouter.h"

namespace MusiCode
{
    /**
     * WebController - 封裝 WebView2 初始化與 JS 通訊邏輯
     */
    class WebController : public juce::Timer
    {
    public:
        WebController(CommandRouter& router) : commandRouter(router)
        {
            // 設定 GPU 參數 (解決黑畫面問題的保險)
            _putenv_s("WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", "--disable-gpu");

            webBrowser = std::make_unique<juce::WebBrowserComponent>(
                juce::WebBrowserComponent::Options{}
                    .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                    .withNativeIntegrationEnabled(true)
                    .withNativeFunction("postToBackend", [this](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion) {
                        if (args.size() > 0)
                            commandRouter.processCommand(args[0]);
                        completion(juce::var("OK"));
                    })
            );

            // 啟動 3 秒禁運期定時器
            startTimer(3000);
        }

        juce::WebBrowserComponent* getBrowser() { return webBrowser.get(); }

        void loadURL(const juce::String& url) {
            webBrowser->goToURL(url);
        }

        /** 向前端發送事件 */
        void sendEvent(const juce::String& eventName, const juce::var& data)
        {
            if (webBrowser != nullptr && !isLoading)
            {
                juce::DynamicObject::Ptr eventObj = new juce::DynamicObject();
                eventObj->setProperty("type", eventName);
                eventObj->setProperty("detail", data);
                
                juce::String json = juce::JSON::toString(eventObj.get());
                juce::String js = "window.dispatchEvent(new CustomEvent('MusiCodeEngineEvent', { detail: " + json + " }));";
                
                juce::MessageManager::callAsync([this, js]() {
                    if (webBrowser != nullptr)
                        webBrowser->goToURL("javascript:" + js);
                });
            }
        }

        void timerCallback() override
        {
            stopTimer();
            isLoading = false;
        }

    private:
        CommandRouter& commandRouter;
        std::unique_ptr<juce::WebBrowserComponent> webBrowser;
        bool isLoading = true;
    };
}
