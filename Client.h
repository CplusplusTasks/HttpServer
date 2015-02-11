#ifndef CLIENT_H
#define CLIENT_H
#include <string>
#include <functional>
#include <vector>
#include "EpollLoop.h"
#include "Socket.h"
#include "Server.h"

namespace MyServer {
    struct Server;

    struct Client : public Socket {
    public:
        const static int EDGE_MODE = 0;
        const static int LEVEL_MODE = 1;

        Client(Client &&);

        virtual void send(const std::string &msg);

        void close();

        virtual bool is_valid() const;

        std::string get_ip() const;

        int read_all(std::vector<char> &);

        ssize_t read_bytes(char *, size_t);

        void set_callback_on_receive(std::function<void()>);

        void set_callback_on_close(std::function<void()>);
//    int read_bytes(std::vector<char>&, int);

        virtual int get_sfd() const;

    private:
        Client(const Client &) = delete;
        Client& operator=(const Client &) = delete;
        Client(int, const std::string &, EpollLoop *, Server *);

        int sfd;
        bool mode;
        bool valid;
        std::string ip;
        std::function<void()> callback_on_receive;
        std::function<void()> callback_on_close;
        EpollLoop *epoll;
        Server *parent;

        bool if_status(int) const;

        virtual void on_receive();

        virtual void notify_all();

        virtual void close_notify();

        virtual void set_valid(bool);

        friend struct Server;
    };
}
#endif
