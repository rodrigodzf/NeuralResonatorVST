#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "server_ws.hpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using ConnectionPtr = std::shared_ptr<WsServer::Connection>;
using MessagePtr = std::shared_ptr<WsServer::InMessage>;

class ServerThread : public juce::Thread, public juce::Thread::Listener
{

public:
    ServerThread(unsigned short port = 8000);
    ~ServerThread();

    void run() override;

    void setOnNewShapeCallback(
        std::function<void(const juce::Path &)> callback);

    void setOnNewMaterialCallback(
        std::function<void(const std::vector<float> &)> callback);

    void setOnNewPositionCallback(
        std::function<void(const std::vector<float> &)> callback);

    void sendMessage(const juce::String &message);

private:
    WsServer mServer;
    std::vector<ConnectionPtr> mConnections;
    
    void exitSignalSent() override;
    void onMessage(
        ConnectionPtr connection,
        MessagePtr in_message
    );
 
    void onOpen(ConnectionPtr connection);
    void onClose(
        ConnectionPtr connection,
        int status, 
        const juce::String &reason
    );

    std::function<void(const juce::Path &)> mOnNewShapeCallback;
    std::function<void(const std::vector<float> &)> mOnNewMaterialCallback;
    std::function<void(const std::vector<float> &)> mOnNewPositionCallback;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ServerThread)
};