#include "ServerThread.h"
#include "HelperFunctions.h"

ServerThread::ServerThread(
    ParameterSyncerIf *parameterSyncerIf,
    unsigned short port
)
    : juce::Thread("Server Thread")
    , mParameterSyncerIfPtr(parameterSyncerIf)
{
    addListener(this);

    mServer.config.port = port;
    auto &endpoint = mServer.endpoint["^/ui/?$"];

    endpoint.on_message = [this](auto connection, auto in_message)
    { this->onMessage(connection, in_message); };

    endpoint.on_open = [this](auto connection) { this->onOpen(connection); };
    endpoint.on_close =
        [this](auto connection, int status, const std::string &reason)
    { this->onClose(connection, status, reason); };
}

ServerThread::~ServerThread()
{
    // allow 100ms for the thread to stop cleanly
    stopThread(100);
}

void ServerThread::exitSignalSent()
{
    mServer.stop();
    mConnections.clear();
}

void ServerThread::run()
{
    // Start server and receive assigned port when server is listening for
    // requests

    // recursively try to find a port that is not in use
    while (checkPortInUse(mServer.config.port))
    {
        JLOG(
            "WS Server: Port " + juce::String(mServer.config.port) + " in use"
        );
        mServer.config.port++;
    }

    mServer.start(mOnStartCallback);

    // Wait for server to stop
    JLOG("WS Server: Stopped");
}

void ServerThread::onMessage(
    std::shared_ptr<WsServer::Connection> connection,
    std::shared_ptr<WsServer::InMessage> in_message
)
{
    auto out_message = in_message->string();

    // std::cout << "Server: Message received: \"" << out_message << "\" from
    // "
    //   << connection.get() << std::endl;

    auto parsedJson = juce::JSON::parse(out_message);
    auto messageType = parsedJson.getProperty("type", {}).toString();
    if (mParameterSyncerIfPtr == nullptr)
    {
        JLOG("Server: TorchWrapperIf is null");
        return;
    }
    if (messageType == "new_parameter")
    {
        JLOG("Server: received new parameter");

        mParameterSyncerIfPtr->receivedParameterChange(parsedJson);
    }
    else if (messageType == "new_shape")
    {
        JLOG("Server: received new shape");

        mParameterSyncerIfPtr->receivedNewShape(parsedJson);
    }
    else if (messageType == "update_shape")
    {
        JLOG("Server: received shape update");

        mParameterSyncerIfPtr->receivedShapeUpdate(parsedJson);
    }

#if 0
    auto parsedJson = JSON::parse(out_message);
    auto messageType = parsedJson.getProperty("type", {}).toString();

    if (mTorchWrapperIf == nullptr)
    {
        JLOG("Server: TorchWrapperIf is null");
        return;
    }
    // TODO: this probably should be in another thread
    if (messageType == "new_shape")
    {
        JLOG("Server: New shape requested");
        juce::Path path;
        int res = 64;
        parsedJson.getProperty("shape", {}).toString();
        if (auto positions =
                parsedJson.getProperty("shape", var()).getArray())
        {
            // get number of positions
            auto numPositions = positions->size();

            for (int i = 0; i < numPositions; i++)
            {
                auto position = (*positions)[i];
                auto x = float(position.getProperty("x", var()));
                auto y = float(position.getProperty("y", var()));

                // the positions are in the range [-1, 1], so we need to
                // scale them to the range [0, res]
                // and flip the y axis
                x = (x + 1) * 0.5 * res;
                y = res - ((y + 1) * 0.5 * res);

                // start a new subpath if this is the first position
                if (i == 0) { path.startNewSubPath(x, y); }
                else { path.lineTo(x, y); }
            }

            // close the subpath
            path.closeSubPath();

            // send to the torch wrapper
            mTorchWrapperIf->receivedNewShape(path);
        }
    }
    else if (messageType == "new_material")
    {
        // get material
        auto material = parsedJson.getProperty("material", {});

        // get density, stiffness, poisson ratio, alpha, and beta
        std::vector<float> materialProperties = {
            float(material.getProperty("density", var())),
            float(material.getProperty("stiffness", var())),
            float(material.getProperty("pratio", var())),
            float(material.getProperty("alpha", var())),
            float(material.getProperty("beta", var()))};

        mTorchWrapperIf->receivedNewMaterial(materialProperties);
    }
    else if (messageType == "new_position")
    {
        // get position
        auto position = parsedJson.getProperty("position", {});

        // get x, y, and z
        std::vector<float> positionProperties = {
            float(position.getProperty("x", var())),
            float(position.getProperty("y", var()))};

        mTorchWrapperIf->receivedNewPosition(positionProperties);
    }

    // std::cout << "Server: Sending message \"" << out_message << "\" to " <<
    // connection.get() << std::endl;
#endif
}

void ServerThread::onOpen(ConnectionPtr connection)
{
    std::stringstream ss;
    ss << connection.get();

    JLOG("Server: Opened connection " + ss.str());

    // add connection to the list of active connections
    mConnections.push_back(connection);

    mParameterSyncerIfPtr->onOpen();
}

void ServerThread::onClose(
    ConnectionPtr connection,
    int status,
    const juce::String &reason
)
{
    std::stringstream ss;
    ss << connection.get();

    // JLOG("Server: Closed connection " + ss.str() +
    //  " with status code " + juce::String(status));
    // remove connection from the list of active connections
    auto it = std::find(mConnections.begin(), mConnections.end(), connection);
    if (it != mConnections.end()) { mConnections.erase(it); }
}

ServerThreadIf *ServerThread::getServerThreadIfPtr()
{
    return this;
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
                    JLOG("Server: Error sending message. " + ec.message());
                }
            }
        );
    }
}

void ServerThread::setOnStartCallback(
    std::function<void(unsigned short)> callback
)
{
    mOnStartCallback = callback;
}

bool ServerThread::checkPortInUse(unsigned short port)
{
    using namespace asio;
    using ip::tcp;

    io_service svc;
    tcp::acceptor a(svc);

    asio::error_code ec;
    a.open(tcp::v4(), ec) || a.bind({tcp::v4(), port}, ec);

    return ec == error::address_in_use;
}