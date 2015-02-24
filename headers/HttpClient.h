#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_h

#include "TcpClient.h"
#include "EpollLoop.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

namespace network {
    struct HttpServer;

    struct HttpClient {
    public:
        HttpClient(HttpClient &&);

        void set_callback_on_receive(std::function<void()>);
        void set_callback_on_close(std::function<void()>);
        void send(HttpResponse const& response) const;
        int get_sfd() const;

        HttpRequest get_next_request();

    private:
        std::function<void()> callback_on_receive;
        std::function<void()> callback_on_close;
        std::vector<char> remaining_characters;
        std::queue<HttpRequest> requests;
        TcpClient *client;

        HttpClient(TcpClient *);

        HttpClient(const HttpClient &) = delete;

        HttpClient &operator=(const HttpClient &) = delete;

        void inner_callback_on_receive();

        bool is_CRLF(int id);

        int get_content_length(int last);

        friend struct HttpServer;
    };
}


#endif