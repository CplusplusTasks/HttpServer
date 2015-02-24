#include "HttpClient.h"
#include <algorithm>
#include <functional>

using namespace network;
using namespace std;

HttpClient::HttpClient(HttpClient&& other_client)
{
    client = other_client.client;
}

HttpClient::HttpClient(TcpClient* client) :
    client(client)
{}

void HttpClient::set_callback_on_receive(function<void()> callback_on_receive) {
    this->callback_on_receive = callback_on_receive;
    client->set_callback_on_receive(std::bind(&HttpClient::inner_callback_on_receive, this));
}

void HttpClient::set_callback_on_close(function<void()> callback_on_close) {
    this->callback_on_close = callback_on_close;
    client->set_callback_on_receive(callback_on_close);
}

bool HttpClient::is_CRLF(int id) {
    int size = (int) remaining_characters.size();
    return id < size && remaining_characters[id] == '\r' &&
           id + 1 < size && remaining_characters[id + 1] == '\n';
}

int HttpClient::get_content_length(int last) {
    string pattern = "Content-Length:";
    int p_sz = (int) pattern.size();
    for (int i = 0; i < last - p_sz; i++) {
        bool flag = true;
        for (int j = i; j < i + p_sz; j++) {
            if (remaining_characters[j] != pattern[j - i]) {
                flag = false;
                break;
            }
        }

        if (flag) {
            string result = "";
            for (int j = i + p_sz; remaining_characters[j] != '\r'; j++) {
                result.push_back(remaining_characters[j]);
            }
            return std::stoi(result);
        }
    }
    return -1;
}

// INV: remaining_characters doesn't contain whole header
void HttpClient::inner_callback_on_receive() {
    int first_character = (int) remaining_characters.size();
    client->read_all(remaining_characters);
    int new_sz = (int) remaining_characters.size();

    while (is_CRLF(first_character)) {
        first_character += 2;
    }

    bool request_is_ready = false;
    // INV, but previous remaining_characters could end like this: CRLF CR
    // and now i could receive LF, and have to check previous 3 symbols
    first_character = first_character > 2 ? first_character - 3 : 0;
    while (first_character < new_sz) {
         if (is_CRLF(first_character) && is_CRLF(first_character + 2)) {
             first_character += 4;
             int content_length = get_content_length(first_character);
             if (content_length > 0) {
                 first_character += content_length;
             }
             request_is_ready = true;
             break;
         }
         first_character++;
    }

    if (request_is_ready) {
        HttpRequest temp = HttpRequest(remaining_characters, first_character);
        requests.push(std::move(temp));

        while (is_CRLF(first_character)) {
            first_character += 2;
        }
        remaining_characters = vector<char>(remaining_characters.begin() + first_character, remaining_characters.end());
        callback_on_receive();
    }
}

HttpRequest HttpClient::get_next_request() {
    HttpRequest temp = std::move(requests.front());
    requests.pop();
    return temp;
}

void HttpClient::send(HttpResponse const& response) const {
    client->send(response.toString());
}

int HttpClient::get_sfd() const {
    return client->get_sfd();
}
