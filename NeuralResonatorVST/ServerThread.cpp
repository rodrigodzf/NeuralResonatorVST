#include "ServerThread.h"

ServerThread::ServerThread(unsigned short port)
    : Thread ("Server Thread")
{
    mServer.config.port = port;
    startThread();
}

ServerThread::~ServerThread()
{
    // allow 1000ms for the thread to stop cleanly
    mServer.stop();
    stopThread(1000);
}

void ServerThread::run()
{
    auto &endpoint = mServer.endpoint["^/ui/?$"];

    endpoint.on_message = [this](auto connection, auto in_message) {
        this->onMessage(connection, in_message);
    };

    endpoint.on_open = [this](auto connection) {
        this->onOpen(connection);
    };

    // Start server and receive assigned port when server is listening for requests
    mServer.start([](unsigned short port) {
            std::cout << "Server: Listening on port " << port << std::endl;
    });

    // Wait for server to stop
    std::cout << "Server: Waiting for server to stop" << std::endl;

}

void ServerThread::onMessage(
    std::shared_ptr<WsServer::Connection> connection,
    std::shared_ptr<WsServer::InMessage> in_message
)
{
    auto out_message = in_message->string();

    std::cout << "Server: Message received: \"" << out_message << "\" from " << connection.get() << std::endl;

    // std::cout << "Server: Sending message \"" << out_message << "\" to " << connection.get() << std::endl;

    // auto send_stream = std::make_shared<WsServer::SendStream>();
    // *send_stream << out_message;

    // // connection->send is an asynchronous function
    // connection->send(send_stream, [](const SimpleWeb::error_code &ec) {
    //     if(ec) {
    //         std::cout << "Server: Error sending message. " <<
    //         // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
    //         "Error: " << ec << ", error message: " << ec.message() << std::endl;
    //     }
    // });
}

void ServerThread::onOpen(std::shared_ptr<WsServer::Connection> connection)
{
    std::cout << "Server: Opened connection " << connection.get() << std::endl;
}