#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "TcpServer.h"
#include "TcpClient.h"
#include "EpollLoop.h"
#include "HttpClient.h"
#include "HttpResponse.h"
#include <string>
#include <memory>

namespace network {
    struct HttpServer {
    public:
        HttpServer(EpollLoop *epoll, std::string addr = "", std::string port = "");

        void set_callback_on_receive(std::function<void(HttpClient *)> forall_callback_on_receive);
        void set_callback_on_accept(std::function<void(HttpClient *)> forall_callback_on_accept);
        void set_callback_on_close(std::function<void(HttpClient *)> forall_callback_on_close);

        void send(HttpResponse const& response) const;

    private:
        std::list<HttpClient> clients;
        std::unordered_map<int, std::list<HttpClient>::iterator> get_clients_iterator;
        std::function<void(HttpClient *)> forall_callback_on_receive;
        std::function<void(HttpClient *)> forall_callback_on_accept;
        std::function<void(HttpClient *)> forall_callback_on_close;

        TcpServer server;
    };
}

#endif