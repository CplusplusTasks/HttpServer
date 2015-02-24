#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <vector>
#include <string>
#include <map>

namespace network {
    struct HttpClient;
    struct HttpServer;
    struct HttpRequest {
    public:
        HttpRequest(HttpRequest&&) = default;
        HttpRequest& operator=(HttpRequest&&) = default;

        std::string getHeader() const;

        std::string getRequestMethod() const;

        std::string getHostName() const;

        std::string getAcceptLanguage() const;

        std::string getQueryUrl() const;

        std::string getResourcePath() const;

        std::string getExtensionOfResource() const;

        std::map <std::string, std::string> getMapQueriesFromUrl() const;

        void print() const;

        int getPostContentLength() const;

        std::string getPostMessage() const;

        std::map <std::string, std::string> getPostMapQuery() const;

    private:
        HttpRequest(std::vector<char> const &request, std::vector<char>::size_type length);

        void fillOtherFields();

        void splitQuery(std::map <std::string, std::string> &, std::string);

        void checkEndOfHeader(std::vector<char>::size_type &, std::vector<char> const &request, std::vector<char>::size_type length);

        std::string header;
        std::string resourcePath;
        std::string extensionOfResource;
        std::string requestMethod;
        std::string hostName;
        std::string acceptLanguage;
        std::string queryUrl;
        std::map <std::string, std::string> mapQueriesFromUrl;

        int postContentLength;
        std::string postMessage;
        std::map <std::string, std::string> postMapQuery;
        friend struct HttpClient;
        friend struct HttpServer;
    };
}

#endif
