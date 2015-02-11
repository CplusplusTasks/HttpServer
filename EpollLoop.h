#ifndef EpollLoop_H
#define EpollLoop_H

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <sys/epoll.h>

namespace MyServer {
    struct Socket;

    struct EpollLoop {
    public:
        EpollLoop();

        ~EpollLoop();

        void pause_epoll_loop();

        void start();

        void add_to_watching(Socket *);

    private:
        int epfd;
        uint32_t events_mask;
        const static int MAX_EVENTS = 64;
        bool launched;
        epoll_event evlist[MAX_EVENTS], ev;
        std::unordered_map<int, Socket *> get_socket;
        std::unordered_map<int, std::queue<std::string> > get_sending_msg;

        void add_to_send(int, std::string const &);

        void remove_flag_epollout(int);

        void send_msg(int, std::string);

        void close_socket(int);

        friend struct Server;
        friend struct Client;
    };
}
#endif
