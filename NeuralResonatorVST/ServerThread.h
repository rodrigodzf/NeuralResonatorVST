#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "simple_ws_server/server_ws.hpp"

#include "ParameterSyncerIf.h"
#include "ServerThreadIf.h"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using ConnectionPtr = std::shared_ptr<WsServer::Connection>;
using MessagePtr = std::shared_ptr<WsServer::InMessage>;

class ServerThread : public juce::Thread,
                     public juce::Thread::Listener,
                     public ServerThreadIf
{
public:
    ServerThread(ParameterSyncerIf *parameterSyncerIf,
                 unsigned short port = 8000);
    ~ServerThread();

    void run() override;

    void sendMessage(const juce::String &message) override;
    ServerThreadIf *getServerThreadIfPtr() override;

private:
    ParameterSyncerIf *mParameterSyncerIfPtr;

    WsServer mServer;
    std::vector<ConnectionPtr> mConnections;

    void exitSignalSent() override;
    void onMessage(ConnectionPtr connection, MessagePtr in_message);

    void onOpen(ConnectionPtr connection);
    void onClose(ConnectionPtr connection, int status,
                 const juce::String &reason);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ServerThread)
};