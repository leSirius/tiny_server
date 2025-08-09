import  <chrono>;
import <functional>;
import <iostream>;
import <string>;
import http;


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


int main() {
    HttpServer httpServer{};
    httpServer.setHttpCallback(HttpResponseCallback);
    httpServer.start();
}
