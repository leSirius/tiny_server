module;
#include <sys/socket.h>
#include "logMacro.h"

export module http:httpServer;
import <chrono>;
import <functional>;
import <iostream>;
import <memory>;

import log;
import tcp;
import config;
import :httpRequest;
import :httpResponse;
import :httpParser;

namespace http {
    using namespace tcp;
    class HttpResponse;

    export class HttpServer {
    public:
        typedef shared_ptr<ConnectionTCP> TcpConnectionPtr;
        typedef function<void(const HttpRequest &, HttpResponse *)> HttpResponseCallback;

        explicit HttpServer(string_view _ip = config::ADDRESS, int _port = config::PORT,
                            int _thdNum = static_cast<int>(config::CPU_CORES));

        ~HttpServer() = default;

        static void httpDefaultCallBack(const HttpRequest &req, HttpResponse *resp);

        void setHttpCallback(HttpResponseCallback cb);

        void start() const;

        void onConnection(ConnectionTCP::CallbackParam conn);

        void onMessage(ConnectionTCP::CallbackParam conn) const;

        void onRequest(ConnectionTCP::CallbackParam conn, const HttpRequest &request) const;

        void setThreadNums(int thread_nums);

        void activeCloseConn(const weak_ptr<ConnectionTCP> &connection);

    private:
        ServerTCP server;
        bool autoClose{true};
        HttpResponseCallback respCallback;
    };

    HttpServer::HttpServer(const string_view _ip, const int _port, const int _thdNum)
        : server(_ip, _port, _thdNum) {
        LOG_INFO << "listening on " << _ip << ":" << _port;
        server.setConnectCB([this](ConnectionTCP::CallbackParam conn) { this->onConnection(conn); });
        server.setMessageCB([this](ConnectionTCP::CallbackParam conn) { this->onMessage(conn); });
        setHttpCallback([](const HttpRequest &req, HttpResponse *resp) {
            httpDefaultCallBack(req, resp);
        });
    }

    void HttpServer::httpDefaultCallBack(const HttpRequest &req, HttpResponse *resp) {
        resp->setStatusCode(HttpResponse::HttpStatusCode::NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }

    void HttpServer::setHttpCallback(HttpResponseCallback cb) { respCallback = std::move(cb); }

    void HttpServer::start() const { server.start(); }

    void HttpServer::onConnection(ConnectionTCP::CallbackParam conn) {
        const int clientFD = conn->getFD();
        InetAddress peerAddr{};
        getpeername(clientFD, peerAddr.getReinterCC(), peerAddr.getLenPtr());
        LOG_INFO << "HttpServer::OnNewConnection : Add connection "
                << "[ fd#" << clientFD << "-id#" << conn->getID() << " ]"
                << " from " << peerAddr.getAddress() << ":" << peerAddr.getPort();
        if (autoClose) {
            conn->runAfter(
                config::AUTO_CLOSE_TIME, [this, p = weak_ptr(conn)] { activeCloseConn(p); }
            );
        }
    }

    void HttpServer::onMessage(ConnectionTCP::CallbackParam conn) const {
        if (conn->getState() == ConnectionTCP::State::Connected) {
            if (const auto req = parseHttpReq(conn->getRecvContent()); req.has_value()) {
                onRequest(conn, req.value());
            } else {
                conn->sendMsg("HTTP/1.1 400 Bad Request\r\n\r\n");
                conn->handleClose();
            }
        }
    }

    bool equalIgnoreCase(const std::string &a, const std::string &b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(a[i])) !=
                std::tolower(static_cast<unsigned char>(b[i]))) {
                return false;
            }
        }
        return true;
    }

    void HttpServer::onRequest(ConnectionTCP::CallbackParam conn, const HttpRequest &request) const {
        const auto connState = request.getHeader("Connection").value_or("");
        const bool close = equalIgnoreCase(connState, "close") ||
                           (request.getVersion() == HttpRequest::Version::Http10 &&
                            !equalIgnoreCase(connState, "keep-alive"));
        HttpResponse response(close);
        respCallback(request, &response);
        conn->sendMsg(response.toString());
        if (response.isCloseConnection()) { conn->handleClose(); }
    }

    void HttpServer::setThreadNums(const int thread_nums) { server.setThreadNum(thread_nums); }

    void HttpServer::activeCloseConn(const weak_ptr<ConnectionTCP> &connection) {
        if (const TcpConnectionPtr conn = connection.lock()) {
            if (conn->getLastActive() + config::AUTO_CLOSE_TIME < Timestamp::getNow()) {
                println("timer close {} timeout {}", conn->getFD(), config::AUTO_CLOSE_TIME);
                conn->handleClose();
            } else {
                conn->runAfter(config::AUTO_CLOSE_TIME, [this, connection] { this->activeCloseConn(connection); });
            }
        }
    }
}
