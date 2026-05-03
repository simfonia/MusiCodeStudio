#include "HttpServer.h"

HttpServer::HttpServer(juce::Label& statusLabel)
    : juce::Thread("HttpServer"), label(statusLabel)
{
    startThread();
}

HttpServer::~HttpServer()
{
    stopThread(2000);
}

void HttpServer::run()
{
    juce::StreamingSocket listener;
    // 監聽本地端 9001 端口
    if (listener.createListener(9001, "127.0.0.1"))
    {
        while (!threadShouldExit())
        {
            if (auto* client = listener.waitForNextConnection())
            {
                // 耐心等待資料進入
                if (client->waitUntilReady(true, 500))
                {
                    char buffer[2048];
                    int bytesRead = client->read(buffer, sizeof(buffer), false);

                    if (bytesRead > 0)
                    {
                        juce::String request(buffer, (size_t)bytesRead);

                        if (request.contains("transport_play")) {
                            updateStatus("MusiCode Engine: PLAYING", juce::Colours::green);
                        } else if (request.contains("transport_stop")) {
                            updateStatus("MusiCode Engine: STOPPED", juce::Colours::red);
                        }

                        // 統一回應 (包含 CORS 標頭)
                        juce::String response =
                            "HTTP/1.1 200 OK\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
                            "Access-Control-Allow-Headers: Content-Type\r\n"
                            "Content-Type: text/plain\r\n"
                            "Content-Length: 2\r\n"
                            "Connection: close\r\n\r\nOK";

                        client->write(response.toRawUTF8(), response.getNumBytesAsUTF8());
                    }
                }

                // 稍微延遲一下再關閉，確保資料發送完畢
                juce::Thread::sleep(50);
                client->close();
                delete client;
            }
        }
    }
}

void HttpServer::updateStatus(const juce::String& text, juce::Colour color)
{
    juce::MessageManager::callAsync([this, text, color]() {
        label.setText(text, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, color);
    });
}
