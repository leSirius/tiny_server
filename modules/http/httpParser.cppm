module;

export module http:httpParser;
import <iostream>;
import  <string_view>;

import :httpRequest;

using namespace std;

namespace http {
    export optional<HttpRequest> parseHttpReq(const std::string_view httpText);

    enum class ParseState {
        Invalid, // 无效
        InvalidMethod, // 无效请求方法
        InvalidUrl, // 无效请求路径
        InvalidVersion, // 无效的协议版本号
        InvalidHeader, // 无效请求头
        Start, // 解析开始
        Method, // 请求方法
        BeforeUrl, // 请求连接前的状态，需要'/'开头
        InUrl, // url处理
        BeforeUrlParamKey, // URL请求参数键之前
        UrlParamKey, // URL请求参数键    10
        BeforeUrlParamValue, // URL请求参数值之前
        UrlParamValue, // URL请求参数值
        BeforeProtocol, // 协议解析之前
        Protocol, // 协议
        BeforeVersion, // 版本开始前
        Version, // 版本
        // Header,
        HeaderKey,
        // HeaderBeforeColon, // 请求头冒号之前
        HeaderAfterColon, // 请求头冒号
        HeaderValue, // 请求值
        WhenCr, // 遇到一个回车   20
        CrLf, // 回车换行
        CrLfCr, // 回车换行之后的状态
        Body, // 请求体
        Complete // 完成
    };

    constexpr char CR{'\r'};
    constexpr char LF{'\n'};

    optional<HttpRequest> parseHttpReq(const std::string_view httpText) {
        int scanPos = 0;
        int segStart{scanPos};
        int colonOrEqual{};
        int headerValueBegin{};
        auto state = ParseState::Start;
        HttpRequest request{};

        const auto setState = [&state](const ParseState s) { state = s; };
        const auto inRange = [&scanPos, &httpText] { return scanPos < httpText.size(); };
        const auto emptyChar = [](const char c) { return isblank(c) || c == CR || c == LF; };
        const auto sliceText = [&httpText](const int begin, const int end) {
            return httpText.substr(begin, end - begin);
        };

        while (scanPos < httpText.size() && state != ParseState::Invalid &&
               state != ParseState::InvalidMethod && state != ParseState::InvalidUrl &&
               state != ParseState::InvalidHeader && state != ParseState::InvalidVersion &&
               state != ParseState::Complete && inRange()
        ) {
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
                            request.setMethod(matchMethod);
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
                        request.setUrl(string(sliceText(segStart, scanPos)));
                        setState(ParseState::BeforeProtocol);
                    } else if (ch == '?') {
                        request.setUrl(string(sliceText(segStart, scanPos)));
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
                        request.addUrlParam(
                            string(sliceText(segStart, colonOrEqual)),
                            string(sliceText(colonOrEqual + 1, scanPos))
                        );
                        setState(ParseState::BeforeUrlParamKey);
                    } else if (isblank(ch)) {
                        request.addUrlParam(
                            string(sliceText(segStart, colonOrEqual)),
                            string(sliceText(colonOrEqual + 1, scanPos))
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
                        request.setProtocol(string(sliceText(segStart, scanPos)));
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
                        request.setVersion(version);
                        if (version != HttpRequest::Version::Invalid) {
                            setState(ParseState::WhenCr);
                        } else {
                            // println("invalid version: {}", sliceText(segStart, scanPos));
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
                        if (auto conLen = request.getHeader("Content-Length");
                            stoi(conLen.value_or("0")) > 0) {
                            setState(ParseState::Body);
                        } else {
                            setState(ParseState::Complete);
                        }
                    } else {
                        if (scanPos < httpText.size()) {
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
                    //  if (isblank(ch)) { setState(ParseState::Invalid); }
                    if (ch == CR) {
                        request.addheader(
                            string(sliceText(segStart, colonOrEqual)),
                            string(sliceText(headerValueBegin, scanPos))
                        );
                        setState(ParseState::WhenCr);
                    }
                    break;

                case ParseState::Body:
                    request.setBody(string(sliceText(scanPos, httpText.size())));
                    setState(ParseState::Complete);
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
        if (state == ParseState::Complete) {
            return request;
        }
        return nullopt;
    }
}
