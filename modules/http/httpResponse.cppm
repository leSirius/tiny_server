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
            TempMoved = 302,
            BadRequest = 400,
            Forbidden = 403,
            NotFound = 404,
            ServerError = 500
        };

        enum class HttpBodyType {
            HTML_TYPE,
            FILE_TYPE,
        };

        explicit HttpResponse(bool close);

        ~HttpResponse();

        void setStatusCode(HttpStatusCode code);

        void setStatusMessage(string msg);

        void setCloseConnection(bool close);

        void setContentType(string contentType);

        void addHeader(string key, string value);

        void setBody(string b);

        string getMessage();

        [[nodiscard]] bool isCloseConnection() const;

        [[nodiscard]] HttpBodyType getBodyType() const;

        [[nodiscard]] int getFileFD() const;

        void setFileFd(int _fd);

        void setBodyType(HttpBodyType type);

        string beforeBody();

        void setContentLength(int len);

        [[nodiscard]] int getContentLength() const;

    private:
        unordered_map<string, string> headers;
        HttpStatusCode statusCode;
        string statusMessage;
        string body;
        bool closeConnection;
        int contentLength{0};
        int fileFd{-1};
        HttpBodyType bodyType{HttpBodyType::HTML_TYPE};
    };

    HttpResponse::HttpResponse(const bool close): statusCode(HttpStatusCode::Unknown), closeConnection(close) {
    }

    HttpResponse::~HttpResponse() = default;


    void HttpResponse::setStatusCode(const HttpStatusCode code) { statusCode = code; }

    void HttpResponse::setStatusMessage(string msg) { statusMessage = std::move(msg); }

    void HttpResponse::addHeader(string key, string value) {
        headers[std::move(key)] = std::move(value);
    }

    void HttpResponse::setContentType(string contentType) {
        addHeader("Content-Type", std::move(contentType));
    }

    void HttpResponse::setBody(string b) {
        setContentLength(b.size());
        body = std::move(b);
    }

    string HttpResponse::getMessage() { return beforeBody() + body; }

    void HttpResponse::setCloseConnection(const bool close) { closeConnection = close; }

    bool HttpResponse::isCloseConnection() const { return closeConnection; }

    void HttpResponse::setBodyType(const HttpBodyType type) { bodyType = type; }

    HttpResponse::HttpBodyType HttpResponse::getBodyType() const { return bodyType; }

    void HttpResponse::setFileFd(const int _fd) { fileFd = _fd; }

    int HttpResponse::getFileFD() const { return fileFd; }

    void HttpResponse::setContentLength(const int len) { contentLength = len; }

    int HttpResponse::getContentLength() const { return contentLength; }

    string HttpResponse::beforeBody() {
        string message;
        message += "HTTP/1.1 " + to_string(static_cast<int>(statusCode)) + " " + statusMessage + "\r\n";
        message += closeConnection ? "Connection: close\r\n" : "Connection: Keep-Alive\r\n";
        message += ("Content-Length: " + to_string(contentLength) + "\r\n");
        for (const auto &[fst, snd]: headers) {
            message += fst + ": " + snd + "\r\n";
        }
        message += "Cache-Control: no-store, no-cache, must-revalidate\r\n";
        message += "\r\n";
        return message;
    }
}
