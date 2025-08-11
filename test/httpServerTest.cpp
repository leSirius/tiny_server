import <chrono>;
import <functional>;
import <iostream>;
import <string>;
import  <thread>;

import config;
import http;
import basekit;


using namespace std;
using namespace http;
const string html = " <font color=\"red\">This is html!</font> ";

void HttpResponseCallback(const HttpRequest &request, HttpResponse *response) {
    if (request.getMethod() != HttpRequest::Method::GET) {
        response->setStatusCode(HttpResponse::HttpStatusCode::BadRequest);
        response->setStatusMessage("Bad Request");
        response->setCloseConnection(true);
    }
    const string url = request.getUrl();
    if (url == "/") {
        response->setStatusCode(HttpResponse::HttpStatusCode::OK);
        response->setBody(html);
        response->setContentType("text/html");
    } else if (url == "/hello") {
        response->setStatusCode(HttpResponse::HttpStatusCode::OK);
        response->setBody("hello world\n");
        response->setContentType("text/plain");
    } else if (url == "/favicon.ico") {
        response->setStatusCode(HttpResponse::HttpStatusCode::OK);
    } else {
        response->setStatusCode(HttpResponse::HttpStatusCode::NotFound);
        response->setStatusMessage("Not Found");
        response->setBody("Sorry Not Found\n");
        response->setCloseConnection(true);
    }
}

unique_ptr<AsyncLog> asynclog;

void AsyncOutputFunc(const string_view sv) { asynclog->append(sv); }

void AsyncFlushFunc() { AsyncLog::flush(); }


int main() {
    asynclog = std::make_unique<AsyncLog>();
    Logger::setOutput(AsyncOutputFunc);
    Logger::setFlush(AsyncFlushFunc);
    asynclog->start();

    const auto size = std::thread::hardware_concurrency() - 1;
    HttpServer httpServer{config::ADDRESS, config::PORT, static_cast<int>(size)};
    httpServer.setHttpCallback(HttpResponseCallback);
    httpServer.start();
}
