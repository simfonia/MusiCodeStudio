#pragma once
#include <JuceHeader.h>
#include "CommandRouter.h"
#include "TransportController.h"

namespace MusiCode
{
    /**
     * WebController - 封裝 WebView2 初始化與 JS 通訊邏輯。
     * 現在包含了一個 30Hz 的高速定時器，用於同步播放頭位置。
     */
    class WebController : public juce::Timer
    {
    public:
        WebController(CommandRouter& router, TransportController& tc) 
            : commandRouter(router), transportController(tc)
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

            // 啟動定時器 (30Hz = 約 33ms)
            // 同時處理 3 秒禁運期與高速播放頭同步
            startTimer(33);
            startTime = juce::Time::getMillisecondCounter();
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
            auto now = juce::Time::getMillisecondCounter();
            
            // 處理 3 秒啟動禁運期
            if (isLoading && (now - startTime > 3000))
            {
                isLoading = false;
                DBG("WebController: JS Embargo Ended.");
            }

            // --- 實作 30Hz 播放頭同步 ---
            // 修正：不論是否在播放，都發送位置，以便 Seek 時紅線能移動
            if (!isLoading)
            {
                auto pos = transportController.getPosition();
                auto& tempoSeq = transportController.getTransport().edit.tempoSequence;
                
                juce::DynamicObject::Ptr data = new juce::DynamicObject();
                data.get()->setProperty("seconds", pos.inSeconds());
                data.get()->setProperty("beats", tempoSeq.toBeats(pos).inBeats());
                
                sendEvent("playhead_sync", data.get());
            }
        }

    private:
        CommandRouter& commandRouter;
        TransportController& transportController;
        std::unique_ptr<juce::WebBrowserComponent> webBrowser;
        
        bool isLoading = true;
        uint32 startTime = 0;
    };
}
