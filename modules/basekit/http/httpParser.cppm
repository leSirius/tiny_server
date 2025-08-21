module;

export module basekit:httpParser;
import <memory>;

import :httpRequest;

using namespace std;

namespace http {
    constexpr char CR{'\r'};
    constexpr char LF{'\n'};

    export class HttpParser {
    public:
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

        HttpParser();

        ~HttpParser();

        bool rollingParse(string_view httpText);

        unique_ptr<HttpRequest> moveRequest();

        optional<unique_ptr<HttpRequest> > tryExtractReset();

        void resetState();

    private:
        unique_ptr<HttpRequest> request;
        ParseState state;

        void setState(ParseState s);

        bool fallIntoInvalid() const;
    };
}
