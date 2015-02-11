#include "HttpRequest.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>

using namespace MyHttpManager;
using std::string;
using std::isspace;
using std::vector;
using std::cout;
using std::endl;
typedef vector<char> const& request_t;
typedef vector<char>::size_type vector_sz_t;

void HttpRequest::print() const {
    //cout << "header: " << endl << header << endl;
    //cout << "---------" << endl;
    cout << "--------BEGIN PRINT---------" << endl;
    cout << "postMessage: " << postMessage << endl;
    cout << "hostName: " << hostName << endl;
    cout << "queryUrl: " << queryUrl << endl;
    cout << "requestMethod: " << requestMethod << endl;
    cout << "acceptLanguage: " << acceptLanguage << endl;
    cout << "resourcePath: " << resourcePath << endl;
    cout << "extensionOfResource: " << extensionOfResource << endl;
    cout << "postContentLength: " << postContentLength << endl;

    cout << "queryInUrl: " << endl;
    for (std::pair<string, string> pp : mapQueriesFromUrl) {
        cout << pp.first << " = " << pp.second << endl;
    }

    cout << "queryPost: " << endl;
    for (std::pair<string, string> pp : postMapQuery) {
        cout << pp.first << " = " << pp.second << endl;
    }
    cout << "--------END PRINT---------" << endl;
}

string HttpRequest::getHeader() const {
    return header;
}

string HttpRequest::getRequestMethod() const {
    return requestMethod;
}

string HttpRequest::getHostName() const {
    return hostName;
}

string HttpRequest::getAcceptLanguage() const {
    return acceptLanguage;
}

string HttpRequest::getQueryUrl() const {
    return queryUrl;
}

string HttpRequest::getResourcePath() const {
    return resourcePath;
}

std::map<string, string> HttpRequest::getMapQueriesFromUrl() const {
    return mapQueriesFromUrl;
}

int HttpRequest::getPostContentLength() const {
    return postContentLength;
}

string HttpRequest::getPostMessage() const {
    return postMessage;
}

std::map<string, string> HttpRequest::getPostMapQuery() const {
    return postMapQuery;
}

string HttpRequest::getExtensionOfResource() const {
    return extensionOfResource;
}

namespace MyHttpManager {
    void skipWhiteSpace(vector_sz_t& curPos, request_t request) {
        while (curPos < request.size() && isspace(request[curPos])) {
            curPos++;
        }
    }

    string getNextWord(vector_sz_t& curPos, request_t request, char delimiter = ' ') {
        skipWhiteSpace(curPos, request);
        string result = "";
        if (curPos < request.size()) 
            do {
                result.push_back(request[curPos]);
                curPos++;
            } while (curPos < request.size() && !(isspace(request[curPos]) || request[curPos] == delimiter));

        return result;
    }

    void goNextLine(vector_sz_t& curPos, request_t request) {
        while(curPos < request.size() && request[curPos] != '\r') {
            curPos++;
        }
    }

    bool endOfHeader(vector_sz_t curPos, request_t request) {
        if (curPos + 3 < request.size()) {
            if (request[curPos] == '\r' &&
                request[curPos + 1] == '\n' &&
                request[curPos + 2] == '\r' &&
                request[curPos + 3] == '\n')
                return true;
        }
        return false;
    }
}

void HttpRequest::checkEndOfHeader(vector_sz_t& curPos, request_t request) {
    goNextLine(curPos, request);
    if (endOfHeader(curPos, request)) {
        header = string(request.data(), curPos);
        curPos += 4; //skip \r\n\r\n
        if (curPos < request.size()) {
            postMessage = string(request.begin() + curPos, request.end());
            curPos = request.size();
        }    
    } else {
        curPos += 2; //skip \r\n
    }
}

vector<string> split(const string& str, char delim) {
   vector<string> result;
   std::stringstream ss(str);
   string item;
   while (std::getline(ss, item, delim)) {
       result.push_back(item);
   }
   return result; // std::move() ? return value optimization =(
}

void HttpRequest::fillOtherFields() {
    string::iterator questionPos = std::find(queryUrl.begin(), queryUrl.end(), '?');
    resourcePath = queryUrl;
    if (questionPos != queryUrl.end()) {
        string queryInUrl = string(questionPos + 1, queryUrl.end()); 
        //std::cerr << "queryInUrl: " << queryInUrl << std::endl;
        for (string curValue : split(queryInUrl, '&')) {
           vector<string> keyAndValue = split(curValue, '=');
           mapQueriesFromUrl[keyAndValue[0]] = keyAndValue[1];
        }
        resourcePath = string(queryUrl.begin(), questionPos); 
    }
    
    string::reverse_iterator last_dot = std::find(queryUrl.rbegin(), queryUrl.rend(), '.');
    if (last_dot != queryUrl.rend())
        extensionOfResource = string(last_dot.base(), questionPos);
    
    if (!postMessage.empty()) 
        for (string curValue : split(postMessage, '&')) {
           vector<string> keyAndValue = split(curValue, '=');
           postMapQuery[keyAndValue[0]] = keyAndValue[1];
        }
}

// constructor
HttpRequest::HttpRequest(MyServer::Client* client) {
    postContentLength = -1;
    vector<char> request;
    client->read_all(request);

    vector_sz_t curPos = 0;
    requestMethod = getNextWord(curPos, request); 
    queryUrl = getNextWord(curPos, request);

    while (curPos < request.size()) {
        checkEndOfHeader(curPos, request);
        string curWord = getNextWord(curPos, request, ':'); 
        curPos++;
        if (curWord == "Host") {
            hostName = getNextWord(curPos, request);
        } else if (curWord == "Accept-Language") {
            acceptLanguage = getNextWord(curPos, request);
        } else if (curWord == "Content-Length") {
            postContentLength = std::stoi(getNextWord(curPos, request));
        } 
    }
    fillOtherFields();
}

