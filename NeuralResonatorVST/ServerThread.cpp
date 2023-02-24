#include "ServerThread.h"

using namespace juce;

ServerThread::ServerThread(unsigned short port) : Thread("Server Thread")
{
    mServer.config.port = port;
    startThread();
}

ServerThread::~ServerThread()
{
    // allow 1000ms for the thread to stop cleanly
    mServer.stop();
    stopThread(200);
}

void ServerThread::run()
{
    auto &endpoint = mServer.endpoint["^/ui/?$"];

    endpoint.on_message = [this](auto connection, auto in_message)
    { this->onMessage(connection, in_message); };

    endpoint.on_open = [this](auto connection)
    { 
        this->onOpen(connection);
    };
    endpoint.on_close = [this](auto connection, int status, const std::string &reason)
    {
        this->onClose(connection, status, reason);
    };

    // Start server and receive assigned port when server is listening for
    // requests
    mServer.start(
        [](unsigned short port)
        { std::cout << "Server: Listening on port " << port << std::endl; });

    // Wait for server to stop
    std::cout << "Server: Waiting for server to stop" << std::endl;
}

void ServerThread::onMessage(
    std::shared_ptr< WsServer::Connection > connection,
    std::shared_ptr< WsServer::InMessage > in_message)
{
    auto out_message = in_message->string();

    std::cout << "Server: Message received: \"" << out_message << "\" from "
              << connection.get() << std::endl;

    auto parsedJson = JSON::parse(out_message);
    auto messageType = parsedJson.getProperty("type", {}).toString();

    if (messageType == "new_shape" && mOnNewShapeCallback != nullptr)
    {
        MessageManagerLock mml(this);
        if (mml.lockWasGained())
        {
            juce::Logger::writeToLog("Server: New shape requested");

            // TODO: Create a new shape from the message
            juce::Path path;
            int res = 64;
            path.addStar(Point(res / 2.f, res / 2.f), 5, 5.0f, 15.0f, 0.0f);
            mOnNewShapeCallback(path);
        }
    }
    else if (messageType == "new_material" &&
             mOnNewMaterialCallback != nullptr)
    {
        MessageManagerLock mml(this);
        if (mml.lockWasGained())
        {
            juce::Logger::writeToLog("Server: New material requested");
        }
    }

    // std::cout << "Server: Sending message \"" << out_message << "\" to " <<
    // connection.get() << std::endl;
}

void ServerThread::onOpen(ConnectionPtr connection)
{
    std::cout << "Server: Opened connection " << connection.get() << std::endl;

    // add connection to the list of active connections
    mConnections.push_back(connection);
}

void ServerThread::onClose(
    ConnectionPtr connection,
    int status,
    const juce::String &reason)
{
    std::cout << "Server: Closed connection " << connection.get() << " with status code "
              << status << std::endl;

    // remove connection from the list of active connections
    auto it = std::find(mConnections.begin(), mConnections.end(), connection);
    if (it != mConnections.end())
    {
        mConnections.erase(it);
    }
}

void ServerThread::setOnNewShapeCallback(
    std::function< void(const juce::Path &) > callback)
{
    mOnNewShapeCallback = callback;
}

void ServerThread::setOnNewMaterialCallback(
    std::function< void(const juce::Path &) > callback)
{
    mOnNewMaterialCallback = callback;
}

void ServerThread::sendMessage(const juce::String &message)
{
    // send message to all connections
    for (auto &connection : mConnections)
    {
        // connection->send is an asynchronous function
        connection->send(
            message.toStdString(),
            [](const auto &ec)
            {
                if (ec)
                {
                    Logger::writeToLog(
                        "Server: Error sending message. " + ec.message()
                    );
                }
            });
    }
}