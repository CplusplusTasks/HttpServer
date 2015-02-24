#include <HttpServer.h>
#include <iostream>
#include <memory>

using namespace network;
using std::string;

network::HttpServer::HttpServer(EpollLoop *epoll, string addr, string port) :
    server(epoll, addr, port)
{}

void HttpServer::set_callback_on_receive(std::function<void(HttpClient *)> forall_callback_on_receive) {
    this->forall_callback_on_receive = forall_callback_on_receive;
    server.set_callback_on_receive([this](TcpClient* client) {
        HttpClient* cur_http_client = &*get_clients_iterator[client->get_sfd()];
        cur_http_client->inner_callback_on_receive();
    });
}

void HttpServer::set_callback_on_accept(std::function<void(HttpClient *)> forall_callback_on_accept2) {
    this->forall_callback_on_accept = forall_callback_on_accept2;
    server.set_callback_on_accept([this](TcpClient* client) {
        clients.push_back(HttpClient(client));
        HttpClient* cur_http_client = &clients.back();
        get_clients_iterator[client->get_sfd()] = --clients.end();

        if (forall_callback_on_close != NULL) {
            cur_http_client->set_callback_on_close(std::bind(forall_callback_on_close, cur_http_client));
        }

        if (forall_callback_on_receive != NULL)
            cur_http_client->set_callback_on_receive(std::bind(forall_callback_on_receive, cur_http_client));

        forall_callback_on_accept(cur_http_client);
    });
}

// when i wrote std::function<void(HttpClient *)> forall_callback_on_close
// he ask me capture forall_callback_on_close, but i already captured this
void HttpServer::set_callback_on_close(std::function<void(HttpClient *)> forall_callback_on_close2) {
    forall_callback_on_close = forall_callback_on_close2;
    server.set_callback_on_close([this](TcpClient* client) {
        std::list<HttpClient>::iterator cur_http_client = get_clients_iterator[client->get_sfd()];
        forall_callback_on_close(&*cur_http_client);
        clients.erase(cur_http_client);
        get_clients_iterator.erase(client->get_sfd());
    });
}

void HttpServer::send(HttpResponse const& response) const {
    server.send(response.toString());
}
