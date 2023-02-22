#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "server_ws.hpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

class ServerThread : public juce::Thread
{

public:
    ServerThread(unsigned short port = 8000);
    ~ServerThread();

    void run() override;

    void setOnNewShapeCallback(
        std::function<void(const juce::Path &)> callback
    );

private:
    WsServer mServer;
    
    void onMessage(
        std::shared_ptr<WsServer::Connection> connection,
        std::shared_ptr<WsServer::InMessage> in_message
    );
 
    void onOpen(std::shared_ptr<WsServer::Connection> connection);

    std::function<void(const juce::Path &)> mOnNewShapeCallback;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ServerThread)
};