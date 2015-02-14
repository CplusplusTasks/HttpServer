#include "HttpResponse.h"
#include <ctime>
#include <cassert>
#include <vector>

using std::string;
namespace MyHttpManager {
    static string getReason(int code) {
        switch (code) {
            case 100: return "Continue";
            case 101: return "Switching Protocols";
            case 200: return "OK";
            case 201: return "Created";
            case 202: return "Accepted";
            case 203: return "Non-Authoritative Information";
            case 204: return "No Content";
            case 205: return "Reset Content";
            case 206: return "Partial Content";
            case 300: return "Multiple Choices";
            case 301: return "Moved Permanently";
            case 302: return "Found";
            case 303: return "See Other";
            case 304: return "Not Modified";
            case 305: return "Use Proxy";
            case 307: return "Temporary Redirect";
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 402: return "Payment Required";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 406: return "Not Acceptable";
            case 407: return "Proxy Authentication Required";
            case 408: return "Request Time-out";
            case 409: return "Conflict";
            case 410: return "Gone";
            case 411: return "Length Required";
            case 412: return "Precondition Failed";
            case 413: return "Request Entity Too Large";
            case 414: return "Request-URI Too Large";
            case 415: return "Unsupported Media Type";
            case 416: return "Requested range not satisfiable";
            case 417: return "Expectation Failed";
            case 500: return "Internal Server Error";
            case 501: return "Not Implemented";
            case 502: return "Bad Gateway";
            case 503: return "Service Unavailable";
            case 504: return "Gateway Time-out";
            case 505: return "HTTP Version not supported";
            default: throw HttpResponseException("unknown status code: " + std::to_string(code));
        }
    }

    static string getContentType(string extension) {
        if (extension == "js") return "text/javascript";
        if (extension == "css") return "text/css";
        if (extension == "ico") return "image/x-icon";
        if (extension == "html") return "text/html";
        if (extension == "jpg") return "image/jpeg";
        if (extension == "jpeg") return "image/jpeg";
        if (extension == "png") return "image/png";
        if (extension == "json") return "application/json";
        throw HttpResponseException("unknown extension: " + extension);
    }
}

using namespace MyHttpManager;

HttpResponseException::HttpResponseException(std::string msg) :
    runtime_error(msg){}

string HttpResponse::getCurTimeUTC(time_t timer) const {
    assert(timer != -1);
    size_t max_buff_sz = 80;
    std::vector<char> buffer(max_buff_sz);
    tm* timeinfo = gmtime(&timer); 
    strftime(buffer.data(), max_buff_sz, "%a, %d %b %G %T GMT", timeinfo);
    return string(buffer.data());
}

string HttpResponse::toString() const {
    if (!validResponse) {
        if (curTime == 0) 
            curTime = time(NULL);
        if (statusReason.empty())
            statusReason = getReason(statusCode);
        string strTime = getCurTimeUTC(curTime);
        wholeResponse.append("HTTP/1.1 " + std::to_string(statusCode) + " " + statusReason + "\r\n");
        wholeResponse.append("Date: " + strTime + "\r\n");
        wholeResponse.append("Content-Language: en\r\n");
        wholeResponse.append("Content-Type: " + getContentType(extension) + "; charser=utf-8\r\n");
        wholeResponse.append("Content-Length: " + std::to_string(message.size()) + "\r\n\r\n");
        wholeResponse.append(message);
    }

    return wholeResponse;
}

HttpResponse& HttpResponse::setExtension(string extension) {
    this->extension = extension;
    return *this;
}

HttpResponse& HttpResponse::setStatusCode(int statusCode) {
    this->statusCode = statusCode;
    return *this;
}

HttpResponse& HttpResponse::setStatusReason(string statusReason) {
    this->statusReason = statusReason; 
    return *this;
}

HttpResponse& HttpResponse::setMessage(string message) {
    this->message = message;
    return *this;
}

HttpResponse& HttpResponse::setWholeResponse(string wholeResponse) {
    this->wholeResponse = wholeResponse;
    return *this;
}

HttpResponse& HttpResponse::send(MyServer::Socket* socket) {
    socket->send(toString());    
    return *this;
}

HttpResponse::HttpResponse() {
    curTime = 0;
    statusCode = 200;
    extension = "html";
}

