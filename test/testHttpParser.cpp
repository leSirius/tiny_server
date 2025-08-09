
import  <print>;
import <string>;
import <unordered_map>;
import <vector>;
import http;

using namespace std;
using namespace http;
#include <string>
#include <vector>

std::vector<std::string> testCases = {
    // 示例 1: 简单 GET 请求
    "GET /index.html HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "Connection: close\r\n"
    "\r\n",

    // 示例 2: 带 body 的 POST 请求
    "POST /submit-form HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Content-Length: 13\r\n"
    "\r\n"
    "name=Calvin&x=1",

    // 示例 3: 多个 Cookie 头
    "GET / HTTP/1.1\r\n"
    "Host: test.com\r\n"
    "Cookie: user=abc\r\n"
    "Cookie: session=xyz\r\n"
    "Connection: keep-alive\r\n"
    "\r\n",

    // 示例 4: HTTP/1.0 请求，无 Host 头
    "GET /legacy HTTP/1.0\r\n"
    "User-Agent: legacy-client\r\n"
    "\r\n",

    // 示例 5: POST 请求带 JSON body
    "POST /api/data HTTP/1.1\r\n"
    "Content-Length: 27\r\n"
    "Host: api.example.com\r\n"
    "Cookie: user=abc\r\n"
    "Accept: text/html\r\n"
    "Accept: text1/html\r\n"
    "Cookie: session=xyz\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "{ \"name\": \"test\", \"x\": 1 }",

    "GET /hello?a=2 HTTP/1.1\r\n"
    "Host: 127.0.0.1:1234\r\n"
    "Connection: keep-alive\r\n"
    "Cache-Control: max-age=0\r\n"
    "sec-ch-ua: \"Google Chrome\";v=\"113\", \"Chromium\";v=\"113\", \"Not-A.Brand\";v=\"24\"\r\n"
    "sec-ch-ua-mobile: ?0\r\n"
    "sec-ch-ua-platform: \"Linux\"\r\n"
    "Upgrade-Insecure-Requests: 1\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0.0.0 Safari/537.36\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n"
    "Sec-Fetch-Site: none\r\n"
    "Sec-Fetch-Mode: navigate\r\n"
    "Sec-Fetch-User: ?1\r\n"
    "Sec-Fetch-Dest: document\r\n"
    "Accept-Encoding: gzip, deflate, br\r\n"
    "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,zh-TW;q=0.7\r\n"
    "Cookie: username-127-0-0-1-8888=\"2|1:0|10:1681994652|23:username-127-0-0-1-8888|44:Yzg5ZjA1OGU0MWQ1NGNlMWI2MGQwYTFhMDAxYzY3YzU=|6d0b051e144fa862c61464acf2d14418d9ba26107549656a86d92e079ff033ea\"; _xsrf=2|dd035ca7|e419a1d40c38998f604fb6748dc79a10|168199465\r\n"
    "\r\n"
};


void validateRequest(auto &req, const string &raw) {
    std::println("---- Validating Request ----");
    std::println("Raw:\n{}", raw);

    std::println("Method: {}", HttpRequest::methodToString(req.getMethod()));
    std::println("URL: {}", req.getUrl());
    std::println("Protocol: {}", req.getProtocol());

    std::print("Version: ");
    switch (req.getVersion()) {
        case http::HttpRequest::Version::Http10: std::println("HTTP/1.0");
            break;
        case http::HttpRequest::Version::Http11: std::println("HTTP/1.1");
            break;
        default: std::println("Invalid");
    }

    std::println("Headers:");
    for (const auto &[k, v]: req.getHeaderMap()) {
        std::println("  {}: {}", k, v);
    }

    std::println("Body:\n{}", req.getBody());

    // 可选验证: URL 参数
    const auto &params = req.getUrlParamsMap();
    if (!params.empty()) {
        std::println("URL Parameters:");
        for (const auto &[k, v]: params) {
            std::println("  {} = {}", k, v);
        }
    }

    std::println("----------------------------\n");
}

int main() {
    for (auto &test: testCases) {
        if (auto rst = http::parseHttpReq(string_view(test)); rst.has_value()) {
            validateRequest(rst.value(), test);
        } else {
            println("always bad news");
        }
        // break;
    }
}





