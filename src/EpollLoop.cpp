#include <sys/epoll.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include "EpollLoop.h"
#include "TcpServer.h"

#define cur_loc get_cur_loc(__FILE__, __func__, __LINE__)

using namespace std;
using namespace network;

static string get_cur_loc(string file, string fun, int line) {
    return file + ": " + fun + ": " + to_string(line);
}

static void print_error(string msg) {
    perror(msg.c_str());
    throw MyServerException("Something go wrong. See above.");
}

static void safely_close_socket(int sfd, const std::string &from_fun) {
    if (close(sfd) == -1) {
        perror((from_fun + ": safely_close_socket").c_str());
        throw MyServerException("Something go wrong. See above.");
    }
}

EpollLoop::EpollLoop() :
        events_mask(EPOLLIN | EPOLLET) {
    epfd = epoll_create1(0);
    if (epfd == -1)
        print_error(cur_loc + ": epoll_create");
}

void EpollLoop::add_to_send(int sfd, string const &msg) {
    ev.events = events_mask | EPOLLOUT;
    ev.data.fd = sfd;

    if (epoll_ctl(epfd, EPOLL_CTL_MOD, sfd, &ev) == -1)
        print_error(cur_loc + ": epoll_ctl");

    get_sending_msg[sfd].push_back(msg);
}

void EpollLoop::pause_epoll_loop() {
    launched = false;
}

void EpollLoop::remove_flag_epollout(int sfd) {
    ev.events = events_mask;
    ev.data.fd = sfd;

    if (epoll_ctl(epfd, EPOLL_CTL_MOD, sfd, &ev) == -1)
        print_error(cur_loc + ": epoll_ctl");
}

ssize_t EpollLoop::send_msg(int client_socket, string data_for_client) {
//    cerr << "i prepare send msg: " << client_socket << "  " << data_for_client << endl;

    ssize_t len_data_for_client = data_for_client.size();
    ssize_t sent_bytes = send(client_socket, (void *) data_for_client.c_str(), len_data_for_client, MSG_NOSIGNAL);

    if (sent_bytes == -1) {
        safely_close_socket(client_socket, cur_loc);
        print_error(cur_loc);
    }
    return sent_bytes;
}

void EpollLoop::start() {
    TcpSocket *socket;
    uint32_t mask;
    launched = true;

    while (!get_socket.empty() && launched) {
//        cerr << "epoll_wait()" << endl;
        int ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
        if (ready == -1) {
            if (errno == EINTR)
                continue; // Restart if interrupted by signal
            else
                print_error(cur_loc + ": epoll_wait");
        }

        for (int i = 0; i < ready; i++) {
            mask = events_mask;
            if (!get_sending_msg.empty()) {
                mask |= EPOLLOUT;
            }
            socket = get_socket[evlist[i].data.fd];
            if ((evlist[i].events & (EPOLLERR | EPOLLHUP)) ||
                    !(evlist[i].events & mask)) {
                /* An error has occurred on this fd, or the socket is not
               ready for reading (why were we notified then?) */
                cerr << (evlist[i].events & (EPOLLERR | EPOLLHUP)) << endl;
                cerr << "epoll error, go next =)" << endl;
                cerr << socket->get_sfd() << endl;
                socket->close_notify();
                continue;
            } else if (evlist[i].events & EPOLLIN) {
                socket->on_receive();
            } else if (evlist[i].events & EPOLLOUT) {
                //INV: get_sending_msg not empty

                int sfd = evlist[i].data.fd;
                string msg = get_sending_msg[sfd].front();
                get_sending_msg[sfd].pop_front();
                ssize_t sent_bytes = send_msg(sfd, msg);

                if ((decltype(msg)::size_type) sent_bytes < msg.size()) {
                    get_sending_msg[sfd].push_front(msg.substr(sent_bytes));
                }

                if (get_sending_msg[sfd].empty()) {
                    get_sending_msg.erase(sfd);
                    remove_flag_epollout(sfd);
                }
            }
        }
    }
}

void EpollLoop::add_to_watching(TcpSocket * socket) {
    int sfd = socket->get_sfd();
    ev.events = events_mask;
    ev.data.fd = sfd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1)
        print_error(cur_loc + ": epoll_ctl");
    get_socket[sfd] = socket;
}

void EpollLoop::close_socket(int sfd) {
    get_socket.erase(sfd);
}

EpollLoop::~EpollLoop() {
    safely_close_socket(epfd, cur_loc);
}
