#ifndef SOCKET_H
#define SOCKET_H

#include <string>

namespace network {
    struct TcpSocket {
    public:
        virtual void send(const std::string &msg) const = 0;

        virtual int get_sfd() const = 0;

        virtual bool is_valid() const = 0;

    private:
        virtual void on_receive() = 0;

        virtual void set_valid(bool) = 0;

        virtual void notify_all() = 0;

        virtual void close_notify() = 0;

    protected:
        void safely_close_socket(const std::string &msg = "", bool enabled_notify_all = true);

        friend struct EpollLoop;
    };
}

#endif
