#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include <string>
#include <ctime>
#include <unordered_map>
#include "Socket.h"
#include <stdexcept>

namespace MyHttpManager {
    struct HttpResponseException : public std::runtime_error {
        HttpResponseException(std::string);
    };

    struct HttpResponse {
    public:
        HttpResponse();
        std::string toString() const;
        HttpResponse& setStatusCode(int);
        HttpResponse& setStatusReason(std::string);
        HttpResponse& setWholeResponse(std::string);
        HttpResponse& setMessage(std::string);
        HttpResponse& setDate(time_t);
        HttpResponse& setExtension(std::string);
        HttpResponse& send(MyServer::Socket*);

    private:
        std::string getCurTimeUTC(time_t) const;

        std::string message;
        int statusCode;
        bool validResponse;
        std::string extension;
        mutable time_t curTime;
        mutable std::string wholeResponse;
        mutable std::string statusReason;

        std::unordered_map<int, std::string> codeToReason;
        std::unordered_map<std::string, std::string> extToContentType;
    };
}

#endif
