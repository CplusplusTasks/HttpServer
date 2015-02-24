#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <functional>
#include <vector>
#include "EpollLoop.h"
#include "TcpSocket.h"
#include "TcpServer.h"

namespace network {
    struct TcpServer;

    struct TcpClient : public TcpSocket {
    public:
        const static int EDGE_MODE = 0;
        const static int LEVEL_MODE = 1;

        TcpClient(TcpClient &&);

        void send(const std::string &msg) const;

        void close();

        bool is_valid() const;

        std::string get_ip() const;

        int read_all(std::vector<char> &buffer);

        ssize_t read_bytes(char *buff, size_t num);

        void set_callback_on_receive(std::function<void()>);

        void set_callback_on_close(std::function<void()>);

        int get_sfd() const;

    private:
        int sfd;
        bool mode;
        bool valid;
        std::string ip;
        EpollLoop *epoll;
        TcpServer *parent;
        std::function<void()> callback_on_receive;
        std::function<void()> callback_on_close;

        TcpClient(const TcpClient &) = delete;

        TcpClient &operator=(const TcpClient &) = delete;

        bool if_status(int) const;

        void on_receive();

        void notify_all();

        void close_notify();

        void set_valid(bool);

        friend struct TcpServer;

    protected:
        TcpClient(int sfd, const std::string &ip, EpollLoop *epoll, TcpServer *parent);
    };
}
#endif
