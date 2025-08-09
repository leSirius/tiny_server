module;

export module http:httpResponse;
import <string>;
import <unordered_map>;

using namespace std;

namespace http {
    export class HttpResponse {
    public:
        enum class HttpStatusCode {
            Unknown = 1,
            Continue = 100,
            OK = 200,
            BadRequest = 400,
            Forbidden = 403,
            NotFound = 404,
            ServerError = 500
        };

        explicit HttpResponse(bool close);

        ~HttpResponse();

        void setStatusCode(HttpStatusCode code);

        void setStatusMessage(string msg);

        void setCloseConnection(bool close);

        void setContentType(string contentType);

        void addHeader(string key, string value);

        void setBody(string b);

        string toString();

        [[nodiscard]] bool isCloseConnection() const;

    private:
        unordered_map<string, string> headers;
        HttpStatusCode statusCode;
        string statusMessage;
        string body;
        bool closeConnection;
    };


    HttpResponse::HttpResponse(const bool close): statusCode(HttpStatusCode::Unknown), closeConnection(close) {
    }

    HttpResponse::~HttpResponse() = default;

    void HttpResponse::setStatusCode(const HttpStatusCode code) {
        statusCode = code;
    }

    void HttpResponse::setStatusMessage(string msg) {
        statusMessage = std::move(msg);
    }

    void HttpResponse::setCloseConnection(const bool close) {
        closeConnection = close;
    }

    void HttpResponse::setContentType(string contentType) {
        addHeader("Content-Type", std::move(contentType));
    }

    void HttpResponse::addHeader(string key, string value) {
        headers[std::move(key)] = std::move(value);
    }


    void HttpResponse::setBody(string b) {
        body = std::move(b);
    }

    string HttpResponse::toString() {
        string respStr = "HTTP/1.1 " + to_string(static_cast<int>(statusCode)) + " " +
                         statusMessage + "\r\n";
        if (closeConnection) {
            respStr += "Connection: close\r\n";
        } else {
            respStr += "Connection: keep-alive\r\n";
            respStr += "Content-Length: " + to_string(body.size()) + "\r\n";
        }
        for (const auto &[fst, snd]: headers) {
            respStr += fst + ": " + snd + "\r\n";
        }
        respStr += "\r\n";
        respStr += body;
        return respStr;
    }

    bool HttpResponse::isCloseConnection() const {
        return closeConnection;
    }
}
