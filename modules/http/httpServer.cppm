module;
#include <unistd.h>
#include <sys/socket.h>
#include "logMacro.h"

export module http:httpServer;
import <chrono>;
import <functional>;
import <iostream>;
import <memory>;

import basekit;
import config;
import :httpRequest;
import :httpResponse;
import :httpParser;

namespace http {
    using namespace basekit;
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
        const bool closeConn = equalIgnoreCase(connState, "close") ||
                               (request.getVersion() == HttpRequest::Version::Http10 && !equalIgnoreCase(
                                    connState, "keep-alive"));
        HttpResponse response(closeConn);
        respCallback(request, &response);

        if (response.getBodyType() == HttpResponse::HttpBodyType::HTML_TYPE) {
            const auto temp = response.getMessage();
            conn->sendMsg(std::move(temp));
        } else {
            conn->sendMsg(response.beforeBody());
            conn->sendFile(response.getFileFD(), response.getContentLength());
            if (const int ret = close(response.getFileFD()); ret == -1) {
                LOG_ERROR << "Close File Error";
            } else {
                LOG_INFO << "Close File Ok";
            }
        }

        if (response.isCloseConnection()) { conn->handleClose(); }
    }

    void HttpServer::setThreadNums(const int thread_nums) { server.setThreadNum(thread_nums); }

    void HttpServer::activeCloseConn(const weak_ptr<ConnectionTCP> &connection) {
        if (const TcpConnectionPtr conn = connection.lock()) {
            if (conn->getLastActive() + config::AUTO_CLOSE_TIME < Timestamp::getNow()) {
                LOG_INFO << "Timer close fd:" << conn->getFD() <<
                    " timeout: " << config::AUTO_CLOSE_TIME.count() << "s";
                conn->handleClose();
            } else {
                conn->runAfter(config::AUTO_CLOSE_TIME, [this, connection] { this->activeCloseConn(connection); });
            }
        }
    }
}
