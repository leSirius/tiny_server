module;


module basekit;
import :httpRequest;
import <iostream>;
import <memory>;
import <ranges>;
import <unordered_map>;

import :httpParser;
using namespace std;

namespace http {
    // unique_ptr<HttpRequest> parseRequest(const string_view httpText) {
    //     HttpParser parser;
    //     if (const auto rst = parser.rollingParse(httpText); rst) {
    //         return parser.moveRequest();
    //     } else {
    //         return nullptr;
    //     }
    // }

    HttpParser::HttpParser(): request(make_unique<HttpRequest>()), state(ParseState::Start) {
    }

    HttpParser::~HttpParser() = default;

    bool HttpParser::rollingParse(const string_view httpText) {
        int scanPos = 0;
        const auto totalSize = httpText.size();
        int segStart{scanPos};
        int colonOrEqual{};
        int headerValueBegin{};

        const auto inRange = [&scanPos, totalSize] { return scanPos < totalSize; };
        const auto emptyChar = [](const char c) { return isblank(c) || c == CR || c == LF; };
        const auto sliceText = [&httpText](const int begin, const int end) {
            auto sub = httpText.substr(begin, end - begin);
            auto filtered = sub | views::filter([](const unsigned char ch) {
                // safari 浏览器会向请求头插入这个字符，滤掉
                return static_cast<int>(ch) != 28;
            });
            return ranges::to<string>(filtered);
        };

        while (state != ParseState::Invalid && state != ParseState::InvalidMethod &&
               state != ParseState::InvalidUrl && state != ParseState::InvalidHeader &&
               state != ParseState::InvalidVersion && state != ParseState::Complete && inRange()) {
            const char &ch = httpText[scanPos];

            switch (state) {
                case ParseState::Start:
                    if (emptyChar(ch)) {
                    } else {
                        setState(ParseState::Method);
                        segStart = scanPos;
                    }
                    break;

                case ParseState::Method:
                    if (isblank(ch)) {
                        if (const auto matchMethod = HttpRequest::stringToMethod(sliceText(segStart, scanPos));
                            matchMethod != HttpRequest::Method::INVALID) {
                            request->setMethod(matchMethod);
                            setState(ParseState::BeforeUrl);
                        } else {
                            setState(ParseState::InvalidMethod);
                        }
                    }
                    break;

                case ParseState::BeforeUrl:
                    if (ch == '/') {
                        setState(ParseState::InUrl);
                        segStart = scanPos;
                    } else if (!isblank(ch)) {
                        setState(ParseState::InvalidUrl);
                    }
                    break;

                case ParseState::InUrl:
                    if (isblank(ch)) {
                        request->setUrl(sliceText(segStart, scanPos));
                        setState(ParseState::BeforeProtocol);
                    } else if (ch == '?') {
                        request->setUrl(sliceText(segStart, scanPos));
                        setState(ParseState::BeforeUrlParamKey);
                    }
                    break;

                case ParseState::BeforeUrlParamKey:
                    if (emptyChar(ch)) {
                        setState(ParseState::InvalidUrl);
                    } else {
                        setState(ParseState::UrlParamKey);
                        segStart = scanPos;
                    }
                    break;

                case ParseState::UrlParamKey:
                    if (ch == '=') {
                        colonOrEqual = scanPos;
                        setState(ParseState::BeforeUrlParamValue);
                    } else if (emptyChar(ch)) {
                        setState(ParseState::InvalidUrl);
                    }
                    break;

                case ParseState::BeforeUrlParamValue:
                    if (emptyChar(ch)) { setState(ParseState::InvalidUrl); } else {
                        setState(ParseState::UrlParamValue);
                    }
                    break;

                case ParseState::UrlParamValue:
                    if (ch == '&') {
                        request->addUrlParam(
                            sliceText(segStart, colonOrEqual),
                            sliceText(colonOrEqual + 1, scanPos)
                        );
                        setState(ParseState::BeforeUrlParamKey);
                    } else if (isblank(ch)) {
                        request->addUrlParam(
                            sliceText(segStart, colonOrEqual),
                            sliceText(colonOrEqual + 1, scanPos)
                        );
                        setState(ParseState::BeforeProtocol);
                    }
                    break;

                case ParseState::BeforeProtocol:
                    if (emptyChar(ch)) {
                    } else {
                        setState(ParseState::Protocol);
                        segStart = scanPos;
                    }
                    break;

                case ParseState::Protocol:
                    if (ch == '/') {
                        request->setProtocol(sliceText(segStart, scanPos));
                        setState(ParseState::BeforeVersion);
                    }
                    break;

                case ParseState::BeforeVersion:
                    if (isdigit(ch)) {
                        setState(ParseState::Version);
                        segStart = scanPos;
                    } else {
                        setState(ParseState::InvalidVersion);
                    }
                    break;

                case ParseState::Version:
                    if (isdigit(ch) || ch == '.') {
                    } else if (emptyChar(ch)) {
                        const auto verString = sliceText(segStart, scanPos);
                        const auto version = verString == "1.0"
                                                 ? HttpRequest::Version::Http10
                                                 : verString == "1.1"
                                                       ? HttpRequest::Version::Http11
                                                       : HttpRequest::Version::Invalid;
                        request->setVersion(version);
                        if (version != HttpRequest::Version::Invalid) {
                            setState(ParseState::WhenCr);
                        } else {
                            setState(ParseState::InvalidVersion);
                        }
                    }
                    break;

                case ParseState::WhenCr:
                    if (ch == LF) {
                        setState(ParseState::CrLf);
                    } else {
                        setState(ParseState::Invalid);
                    }
                    break;

                case ParseState::CrLf:
                    if (ch == CR) {
                        setState(ParseState::CrLfCr);
                    } else if (isblank(ch)) {
                        setState(ParseState::Invalid);
                    } else {
                        setState(ParseState::HeaderKey);
                        segStart = scanPos;
                    }
                    break;

                case ParseState::CrLfCr:
                    if (ch == LF) {
                        if (auto conLen = request->getHeader("Content-Length");
                            stoi(conLen.value_or("0")) > 0
                        ) {
                            setState(ParseState::Body);
                            segStart = scanPos + 1;
                        } else {
                            setState(ParseState::Complete);
                        }
                    } else {
                        if (scanPos < httpText.size()) {
                            // 这里scanPos已经指向了Body的第一个字符，如果Body只有一个字符，那Body状态就不会处理了，暂且减1
                            segStart = scanPos;
                            scanPos -= 1;
                            setState(ParseState::Body);
                        } else {
                            setState(ParseState::Complete);
                        }
                    }
                    break;

                case ParseState::HeaderKey:
                    if (isblank(ch)) {
                        setState(ParseState::Invalid);
                    } else if (ch == ':') {
                        colonOrEqual = scanPos;
                        setState(ParseState::HeaderAfterColon);
                    }
                    break;

                case ParseState::HeaderAfterColon:
                    if (!isblank(ch)) {
                        headerValueBegin = scanPos;
                        setState(ParseState::HeaderValue);
                    }
                    break;

                case ParseState::HeaderValue:
                    if (ch == CR) {
                        request->addheader(
                            sliceText(segStart, colonOrEqual),
                            sliceText(headerValueBegin, scanPos)
                        );
                        setState(ParseState::WhenCr);
                    }
                    break;

                case ParseState::Body:
                    // 进入此分支时需要保证scanPos指向body第一个字符（也该是segStart的位置），并仅触发一次此分支
                    request->setBody(sliceText(segStart, totalSize));
                    if (totalSize - segStart >=
                        stoi(request->getHeader("Content-Length").value_or("0"))
                    ) {
                        setState(ParseState::Complete);
                    }
                    scanPos = totalSize - 1;
                    break;

                case ParseState::Invalid:
                    break;
                case ParseState::InvalidMethod:
                    break;
                case ParseState::InvalidUrl:
                    break;
                case ParseState::InvalidVersion:
                    break;
                case ParseState::InvalidHeader:
                    break;
                case ParseState::Complete:
                    break;
            }
            scanPos += 1;
        }
        return !fallIntoInvalid();
    }

    unique_ptr<HttpRequest> HttpParser::moveRequest() {
        auto another = make_unique<HttpRequest>();
        std::swap(request, another);
        return std::move(another);
    }

    optional<unique_ptr<HttpRequest> > HttpParser::tryExtractReset() {
        if (state != ParseState::Complete) { return nullopt; }
        setState(ParseState::Start);
        auto newPtr = make_unique<HttpRequest>();
        request.swap(newPtr);
        return std::move(newPtr);
    }

    void HttpParser::resetState() {
        setState(ParseState::Start);
        request = nullptr;
    }

    void HttpParser::setState(const ParseState s) { state = s; }

    bool HttpParser::fallIntoInvalid() const {
        return state == ParseState::Invalid || state == ParseState::InvalidMethod ||
               state == ParseState::InvalidUrl || state == ParseState::InvalidHeader ||
               state == ParseState::InvalidVersion;
    }
}
