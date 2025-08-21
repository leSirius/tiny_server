module;

export module basekit:httpRequest;
import <algorithm>;
import <array>;
import <optional>;
import <string>;
import <unordered_map>;
#include <unordered_set>
import <vector>;

using namespace std;

namespace http {
    export class HttpRequest {
    public:
        enum class Method {
            INVALID = 0,
            GET,
            POST,
            HEAD,
            PUT,
            DELETE,
            COUNT
        };

        static array<string, static_cast<int>(Method::COUNT)> methodString;

        static string methodToString(Method);

        static Method stringToMethod(string_view);

        enum class Version {
            Invalid,
            Http10,
            Http11,
        };

        void setUrl(string _url);

        [[nodiscard]] const string &getUrl() const;

        void setMethod(Method _method);

        [[nodiscard]] Method getMethod() const;

        void setVersion(Version _version);

        [[nodiscard]] Version getVersion() const;

        void addUrlParam(string _key, string _value);

        optional<const string> getUrlParam(const string &_key);

        const unordered_map<string, string> &getUrlParamsMap();

        void addheader(string key, string value);

        [[nodiscard]] optional<string> getHeader(string_view key) const;

        [[nodiscard]] const unordered_map<string, string> &getHeaderMap() const;

        void setProtocol(string _protocol);

        [[nodiscard]] const string &getProtocol() const;

        void setBody(string _body);

        [[nodiscard]] const string &getBody() const;

    private:
        string url;
        Method method{Method::INVALID};
        Version version{Version::Invalid};
        unordered_map<string, string> urlParams;
        unordered_map<string, string> headers;
        string protocol;
        string body;
    };

    const unordered_set<string> multiValueHeaders = {
        "A-IM",
        "Accept",
        "Accept-Charset",
        "Accept-Encoding",
        "Accept-Language",
        "Access-Control-Request-Headers",
        "Cache-Control",
        "Connection",
        "Content-Encoding",
        "Expect",
        "Forwarded",
        "If-Match",
        "If-None-Match",
        "Range",
        "TE",
        "Trailer",
        "Transfer-Encoding",
        "Upgrade",
        "Via",
        "Warning"
    };

    array<string, static_cast<int>(HttpRequest::Method::COUNT)> HttpRequest::methodString = {
        "INVALID",
        "GET",
        "POST",
        "HEAD",
        "PUT",
        "DELETE",
    };

    string HttpRequest::methodToString(Method m) {
        if (m == Method::COUNT) { throw logic_error("can't map COUNT to string"); }
        const auto ind = static_cast<int>(m);
        return methodString[ind];
    }

    // starts_with 检查，长度不匹配要求分词，匹配要求相同
    HttpRequest::Method HttpRequest::stringToMethod(const string_view s) {
        const auto it = ranges::find_if(methodString, [s](const auto &m) {
            if (s.length() > m.length()) { return s.starts_with(m) && s[m.length()] == ' '; }
            if (s.length() == m.length()) { return s.starts_with(m); }
            return false;
        });
        if (it != methodString.end()) {
            return static_cast<Method>(distance(methodString.begin(), it));
        } else {
            return Method::INVALID;
        }
    }

    void HttpRequest::setUrl(string _url) { url = std::move(_url); }

    const string &HttpRequest::getUrl() const { return url; }

    void HttpRequest::setMethod(const Method _method) { method = _method; }

    HttpRequest::Method HttpRequest::getMethod() const { return method; }

    void HttpRequest::setVersion(const Version _version) { version = _version; }

    HttpRequest::Version HttpRequest::getVersion() const { return version; }

    void HttpRequest::addUrlParam(string _key, string _value) {
        urlParams[std::move(_key)] = std::move(std::move(_value));
    }

    optional<const string> HttpRequest::getUrlParam(const string &_key) {
        if (const auto it = urlParams.find(_key); it != urlParams.end()) {
            return it->second;
        }
        return nullopt;
    }

    const unordered_map<string, string> &HttpRequest::getUrlParamsMap() { return urlParams; }

    void HttpRequest::addheader(std::string key, std::string value) {
        if (auto [it, inserted] = headers.emplace(key, value); !inserted) {
            if (multiValueHeaders.contains(key)) {
                it->second += string(", ") + value;
            } else {
                it->second = std::move(value);
            }
        }
    }

    optional<string> HttpRequest::getHeader(const string_view key) const {
        if (headers.contains(key.data())) {
            return headers.at(key.data());
        }
        return nullopt;
    }

    const unordered_map<string, string> &HttpRequest::getHeaderMap() const { return headers; }

    void HttpRequest::setProtocol(string _protocol) { protocol = std::move(_protocol); }

    const string &HttpRequest::getProtocol() const { return protocol; }

    void HttpRequest::setBody(string _body) { body = std::move(_body); }

    const string &HttpRequest::getBody() const { return body; }
}
