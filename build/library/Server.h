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
#include "Socket.h"
#include "Client.h"

namespace MyServer {
    struct Client;

    struct MyServerException : public std::runtime_error {
        MyServerException(std::string msg);
    };

    struct Server : public Socket {
    public:
        Server(EpollLoop *epoll, std::string addr = "", std::string port = "");

        ~Server();

        Server(const Server &) = delete;

        Server& operator=(const Server &) = delete;

        virtual void send(const std::string &msg) const;

        void shut_down();

        void set_max_number_clients(uint);

        virtual int get_sfd() const;

        virtual bool is_valid() const;

        void set_callback_on_accept(std::function<void(Client *)> forall_callback_on_accept);

        void set_callback_on_receive(std::function<void(Client *)> forall_callback_on_receive);

        void set_callback_on_close(std::function<void(Client *)> forall_callback_on_close);

        std::list<Client> const &getClients() const;

        unsigned long getCountClients() const;

    private:
        uint max_number_clients = 1000;
        int sfd;
        int backlog;
        bool valid;
        std::string addr, port;
        std::string cur_ip;
        std::function<void(Client *)> forall_callback_on_accept;
        std::function<void(Client *)> forall_callback_on_receive;
        std::function<void(Client *)> forall_callback_on_close;
        std::list<Client> clients;
        std::unordered_map<int, std::list<Client>::iterator> get_clients_iterator;
        EpollLoop *epoll;

        virtual void set_valid(bool);

        virtual void on_receive();

        virtual void notify_all();

        void close_client(int);

        int accept_to_socket();

        static int make_socket_non_blocking(int);

        static std::string get_ip_of_client(const sockaddr_in *);

        virtual void close_notify();

        void listen(const char *, const char *);

        friend struct Client;
    };
}

#endif
