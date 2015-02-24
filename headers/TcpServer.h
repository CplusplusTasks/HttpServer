#ifndef SERVER_H
#define SERVER_H

#include <functional>
#include <vector>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <list>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "EpollLoop.h"
#include "TcpSocket.h"
#include "TcpClient.h"
#include <memory>

namespace network {
    struct TcpClient;

    struct MyServerException : public std::runtime_error {
        MyServerException(std::string msg);
    };

    struct TcpServer : public TcpSocket {
    public:
        TcpServer(EpollLoop *epoll, std::string addr = "", std::string port = "");

        ~TcpServer();

        TcpServer(const TcpServer &) = delete;

        TcpServer &operator=(const TcpServer &) = delete;

        void send(const std::string &msg) const;

        void shut_down();

        void set_max_number_clients(uint);

        void listen(const char *addr, const char *port);

        int get_sfd() const;

        bool is_valid() const;

        void set_callback_on_accept(std::function<void(TcpClient *)> forall_callback_on_accept);

        void set_callback_on_receive(std::function<void(TcpClient *)> forall_callback_on_receive);

        void set_callback_on_close(std::function<void(TcpClient *)> forall_callback_on_close);

        size_t getCountClients();

    private:
        std::list <TcpClient> clients;
        uint max_number_clients = 1000;
        int sfd;
        int backlog;
        bool valid;
        std::string addr, port;
        std::string cur_ip;
        std::unordered_map<int, std::list<TcpClient>::iterator> get_clients_iterator;
        EpollLoop *epoll;
        std::function<void(TcpClient *)> forall_callback_on_accept;
        std::function<void(TcpClient *)> forall_callback_on_receive;
        std::function<void(TcpClient *)> forall_callback_on_close;

        void set_valid(bool);

        void on_receive();

        void notify_all();

        void remove_client(int);

        int accept_to_socket();

        static int make_socket_non_blocking(int);

        static std::string get_ip_of_client(const sockaddr_in *);

        void close_notify();

        friend struct TcpClient;
    };
}

#endif
