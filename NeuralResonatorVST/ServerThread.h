#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "server_ws.hpp"

#include "TorchWrapperIf.h"
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using ConnectionPtr = std::shared_ptr<WsServer::Connection>;
using MessagePtr = std::shared_ptr<WsServer::InMessage>;

class ServerThread : public juce::Thread, public juce::Thread::Listener
{

public:
    ServerThread(
        TorchWrapperIf *torchWrapperIf,
        unsigned short port = 8000);
    ~ServerThread();

    void run() override;

    void sendMessage(const juce::String &message);

private:
    TorchWrapperIf *mTorchWrapperIf;

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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ServerThread)
};