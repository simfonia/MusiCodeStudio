#include "HttpServer.h"

HttpServer::HttpServer(juce::Label& statusLabel, AudioEngine& engine)
    : juce::Thread("HttpServer"), label(statusLabel), audioEngine(engine)
{
    router = std::make_unique<CommandRouter>(engine, [this](const juce::String& text, juce::Colour color) {
        updateStatus(text, color);
    });
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
                    handleRequest(client);
                }

                // 稍微延遲一下再關閉，確保資料發送完畢
                juce::Thread::sleep(50);
                client->close();
                delete client;
            }
        }
    }
}

void HttpServer::handleRequest(juce::StreamingSocket* client)
{
    char buffer[4096];
    int bytesRead = client->read(buffer, sizeof(buffer), false);

    if (bytesRead > 0)
    {
        juce::String request(buffer, (size_t)bytesRead);

        // 簡單提取 JSON Body (尋找第一個 {)
        int bodyStart = request.indexOf("{");
        if (bodyStart != -1)
        {
            juce::String jsonBody = request.substring(bodyStart);
            router->processCommand(jsonBody);
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

void HttpServer::updateStatus(const juce::String& text, juce::Colour color)
{
    juce::MessageManager::callAsync([this, text, color]() {
        label.setText(text, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, color);
    });
}
