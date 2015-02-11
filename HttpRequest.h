#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include <vector>
#include <string>
#include <map>
#include "Client.h"

namespace MyHttpManager {
    struct HttpRequest {
    public:
        HttpRequest(MyServer::Client*);

        std::string getHeader() const;
        std::string getRequestMethod() const;
        std::string getHostName() const;
        std::string getAcceptLanguage() const;
        std::string getQueryUrl() const;
        std::string getResourcePath() const;
        std::string getExtensionOfResource() const;
        std::map<std::string, std::string> getMapQueriesFromUrl() const;
        void print() const;

        int getPostContentLength() const;
        std::string getPostMessage() const;
        std::map<std::string, std::string> getPostMapQuery() const;

        HttpRequest& operator=(HttpRequest const&) = default;
        
    private:
        void fillOtherFields();
        void checkEndOfHeader(std::vector<char>::size_type&, std::vector<char> const& request);

        std::string header;
        std::string resourcePath;
        std::string extensionOfResource;
        std::string requestMethod;
        std::string hostName;
        std::string acceptLanguage;
        std::string queryUrl;
        std::map<std::string, std::string> mapQueriesFromUrl;

        int postContentLength;
        std::string postMessage;
        std::map<std::string, std::string> postMapQuery;
    };
}

#endif
