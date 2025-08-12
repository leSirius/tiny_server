#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "logMacro.h"

import <chrono>;
import <filesystem>;
import <fstream>;
import <functional>;
import <iostream>;
import <string>;
import <thread>;


import config;
import http;
import basekit;


using namespace std;
using namespace http;

const string staticPath = "/tmp/tmp.UveHWpFlDa/tiny_server/static/";
const string downloadPath = staticPath + "download/";
const string fileListSlot = "<!--filelist-->";

string readFile(const string_view path) {
    ifstream file(path.data(), ifstream::in);
    return string(istreambuf_iterator(file), istreambuf_iterator<char>());
}

string getFileList() {
    namespace fs = std::filesystem;
    const auto &filePath = downloadPath;
    string fileList{};
    try {
        for (const auto &dir_entry: fs::recursive_directory_iterator(filePath)) {
            const auto fileName = fs::relative(dir_entry.path(), filePath).string();
            fileList += string("<tr><td>") + fileName + "</td>" +
                    "<td>" +
                    "<a href=\"/download/" + fileName + "\">下载</a>" + " / "
                    "<a href=\"/delete/" + fileName + "\">删除</a>" +
                    "</td></tr>" + "\n";
        }
    } catch (const exception &e) {
        println("filesystem error: {}", e.what());
    }
    return fileList;
}

void downloadFile(const string &filename, HttpResponse *response) {
    if (const int fileFD = open((downloadPath + filename).c_str(), O_RDONLY); fileFD != -1) {
        struct stat fileStat{};
        fstat(fileFD, &fileStat);
        response->setStatusCode(HttpResponse::HttpStatusCode::OK);
        response->setContentLength(static_cast<int>(fileStat.st_size));
        response->setContentType("application/octet-stream");
        response->setBodyType(HttpResponse::HttpBodyType::FILE_TYPE);
        response->setFileFd(fileFD);
    } else {
        LOG_ERROR << "OPEN FILE ERROR";
        response->setStatusCode(HttpResponse::HttpStatusCode::TempMoved);
        response->setStatusMessage("Moved Temporarily");
        response->setContentType("text/html");
        response->addHeader("Location", "/");
    }
}

void removeFile(const string &filename) {
    if (const int ret = remove(string(downloadPath + filename).c_str()); ret != 0) {
        LOG_ERROR << "删除文件 " << filename << " 失败";
    }
}

void HttpResponseCallback(const HttpRequest &request, HttpResponse *response) {
    const string url = request.getUrl();
    if (request.getMethod() == HttpRequest::Method::GET) {
        if (url == "/") {
            string templateHtml = readFile(staticPath + "index.html");
            const auto hydrateHtml = templateHtml.replace(
                templateHtml.find(fileListSlot), fileListSlot.size(), getFileList()
            );
            response->setStatusCode(HttpResponse::HttpStatusCode::OK);
            response->setBody(hydrateHtml);
            response->setContentType("text/html");
        } else if (url == "/hello") {
            response->setStatusCode(HttpResponse::HttpStatusCode::OK);
            response->setBody("hello world\n");
            response->setContentType("text/plain");
        } else if (url == "/favicon.ico") {
            response->setStatusCode(HttpResponse::HttpStatusCode::OK);
        } else if (url == "/json") {
            response->setStatusCode(HttpResponse::HttpStatusCode::OK);
            response->setBody(R"({
                 "user": "Jane Doe",  "age": 30, "isStudent": false, "courses": ["C++", "Python", "JavaScript"]
            })");
            response->setContentType("application/json");
        } else if (url.substr(0, 9) == "/download") {
            downloadFile(url.substr(10), response);
        } else if (url.substr(0, 7) == "/delete") {
            removeFile(url.substr(8));
        } else {
            response->setStatusCode(HttpResponse::HttpStatusCode::NotFound);
            response->setStatusMessage("Not Found");
            response->setBody("Sorry Not Found\n");
            response->setCloseConnection(true);
        }
    } else if (request.getMethod() == HttpRequest::Method::POST) {
        if (url == "/login") {
            const string &reqBody = request.getBody();
            int usernamePos = reqBody.find("username=");
            int passwordPos = reqBody.find("password=");
            usernamePos += 9;
            passwordPos += 9;
            const size_t usernameEndPos = reqBody.find('&', usernamePos);
            const size_t passwordEndPos = reqBody.length();
            const string username = reqBody.substr(usernamePos, usernameEndPos - usernamePos);
            const string password = reqBody.substr(passwordPos, passwordEndPos - passwordPos);

            LOG_INFO << username + " " + password;
            if (username == "sirius" && password == "123456") {
                response->setBody(username + " login ok!\n");
            } else {
                response->setBody("error!\n");
            }
            response->setStatusCode(HttpResponse::HttpStatusCode::OK);
            response->setStatusMessage("OK");
            response->setContentType("text/plain");
        }
    }
}


unique_ptr<AsyncLog> asynclog;

void AsyncOutputFunc(const string_view sv) { asynclog->append(sv); }

void AsyncFlushFunc() { AsyncLog::flush(); }


int main() {
    // asynclog = std::make_unique<AsyncLog>();
    // Logger::setOutput(AsyncOutputFunc);
    // Logger::setFlush(AsyncFlushFunc);
    // asynclog->start();

    const auto size = std::thread::hardware_concurrency() - 1;
    HttpServer httpServer{config::ADDRESS, config::PORT, static_cast<int>(size)};
    httpServer.setHttpCallback(HttpResponseCallback);
    httpServer.start();
}
