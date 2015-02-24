#include <unistd.h>
#include "TcpServer.h"
#define cur_loc get_cur_loc(__FILE__, __func__, __LINE__)
#ifndef NDEBUG
#define check_on_valid  if (!valid) throw MyServerException((string) "Attempt to run function: " + __PRETTY_FUNCTION__ + ", but client's socket is invalid.");
#else
#define check_on_valid
#endif

using namespace std;
using namespace network;

namespace network {
    static string get_cur_loc(string file, string fun, int line) {
        return file + ": " + fun + ": " + to_string(line);
    }

    static void print_error(string msg) {
        perror(msg.c_str());
        throw MyServerException("Something go wrong. See above.");
    }
}

TcpServer::TcpServer(EpollLoop *epoll, std::string addr, std::string port) :
        backlog(10),
        valid(true),
        addr(addr),
        port(port),
        epoll(epoll),
        forall_callback_on_accept(NULL),
        forall_callback_on_receive(NULL),
        forall_callback_on_close(NULL)
{
    if (port.empty()) {
        throw MyServerException("Port must be non empty!");
    }
    listen(addr.empty() ? NULL : addr.c_str(), port.c_str());
}

void TcpServer::listen(const char *addr, const char *port) {
    addrinfo hints;
    addrinfo *serv_info;
    addrinfo *p = NULL;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; //now is any; //commented - loopback address

    int state_getaddr = getaddrinfo(addr, port, &hints, &serv_info);
    if (state_getaddr != 0) {
        throw MyServerException(cur_loc + ": getaddrinfo: \n" + gai_strerror(state_getaddr));
    }

    for (p = serv_info; p != NULL; p = p->ai_next) {
        sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sfd == -1) {
            perror(cur_loc.c_str());
            continue;
        }

        // lose the pesky "Address already in use" error message
        int yes = 1;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            if (::close(sfd) == -1) {
                perror(cur_loc.c_str());
                throw MyServerException("Something go wrong. See above.");
            }
            break;
        }

        if (bind(sfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror((cur_loc + ": bind").c_str());
            if (::close(sfd) == -1) {
                perror(cur_loc.c_str());
                throw MyServerException("Something go wrong. See above.");
            }
            continue;
        }

        break;
    }

    if (p == NULL) {
        throw MyServerException("can't bind address: " + cur_loc);
    }
    freeaddrinfo(serv_info);

    make_socket_non_blocking(sfd);

    if (::listen(sfd, backlog) == -1) {
        print_error(cur_loc + "listen");
    }

    epoll->add_to_watching(this);
}

void TcpServer::send(const std::string &msg) const {
    check_on_valid
    for (TcpClient const &c : clients) {
        c.send(msg);
    }
}

int TcpServer::accept_to_socket() {
    sockaddr_storage client_info;
    socklen_t size_client_info = sizeof client_info;

    int client_socket = accept(sfd, (sockaddr *) &client_info, &size_client_info);
    if (client_socket == -1) {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {

            /* We have processed all incoming connections. */
            return -1;
        } else {
            perror((cur_loc + ": accept").c_str());
            return -1;
        }
    }
    cur_ip = get_ip_of_client((sockaddr_in *) &client_info);
    return client_socket;
}

int TcpServer::make_socket_non_blocking(int sfd) {
    int flags, s;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        perror((cur_loc + ": fcntl").c_str());
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) {
        perror((cur_loc + ": fcntl").c_str());
        return -1;
    }

    return 0;
}

//add exception
void TcpServer::on_receive() {
    while (true) {
        if (clients.size() == max_number_clients) {
            throw MyServerException("Too many clients. More then max_number_clients");
        }

        /* We have a notification on the listening socket. It can store more than one clients */
        int infd = accept_to_socket();
        if (infd == -1)
            break;

        int s = make_socket_non_blocking(infd);
        if (s == -1)
            print_error(cur_loc + ": make_socket_non_blocking");

        clients.push_back(TcpClient(infd, cur_ip, epoll, this));
        get_clients_iterator[infd] = (--clients.end());

        TcpClient *client = &clients.back();

        if (client->is_valid()) {
            epoll->add_to_watching(client);
        } else {
            clients.pop_back();
            return;
        }

        if (forall_callback_on_receive != NULL)
            client->set_callback_on_receive(std::bind(forall_callback_on_receive, client));

        if (forall_callback_on_close != NULL)
            client->set_callback_on_close(std::bind(forall_callback_on_close, client));

        if (forall_callback_on_accept != NULL) {
            forall_callback_on_accept(client);
        }

        if (!valid) return;
    }

    //clients
}

TcpServer::~TcpServer() {
    close_notify();
}

void TcpServer::close_notify() {
    if (!valid) return;
    safely_close_socket(cur_loc);
    clients.clear();
    get_clients_iterator.clear();
    forall_callback_on_receive = forall_callback_on_close = forall_callback_on_accept = NULL;
    addr.clear();
    port.clear();
    epoll = NULL;
}

//INV: server is valid
void TcpServer::notify_all() {
    for (TcpClient &c : clients) {
        if (c.is_valid()) {
            c.safely_close_socket(cur_loc, false);
            epoll->close_socket(c.get_sfd());
        }
    }
    epoll->close_socket(sfd);
}

// do refactor, remove client
void TcpServer::close_client(int sfd) {
    #ifndef NDEBUG
        if (!get_clients_iterator.count(sfd)) return;
    #endif
    clients.erase(get_clients_iterator[sfd]);
    get_clients_iterator.erase(sfd);
}

void TcpServer::set_max_number_clients(uint m) {
    max_number_clients = m;
}

void TcpServer::set_valid(bool valid) {
    this->valid = valid;
}

bool TcpServer::is_valid() const {
    return valid;
}

int TcpServer::get_sfd() const {
    return sfd;
}

string TcpServer::get_ip_of_client(const sockaddr_in *client_info) {
    char ip_of_client[INET_ADDRSTRLEN];
    const sockaddr_in *serv_addr = client_info;
    const in_addr *info_addr = &(serv_addr->sin_addr);

    const char *dst = inet_ntop(serv_addr->sin_family, info_addr, ip_of_client, sizeof ip_of_client);
    if (dst == NULL) {
        perror((cur_loc + ": inet_ntop").c_str()); // do normal log
        return "";
    } else {
        //cerr << "ip of new client is: " << ip_of_client << "; his id: " << get_client_id[sfd] << " sfd: " << sfd << endl;
        return ip_of_client;
    }
}

void TcpServer::set_callback_on_accept(std::function<void(TcpClient *)> forall_callback_on_accept) {
    this->forall_callback_on_accept = forall_callback_on_accept;
}

void TcpServer::set_callback_on_receive(std::function<void(TcpClient *)> forall_callback_on_receive) {
    this->forall_callback_on_receive = forall_callback_on_receive;
}

void TcpServer::set_callback_on_close(std::function<void(TcpClient *)> forall_callback_on_close) {
    this->forall_callback_on_close = forall_callback_on_close;
}

MyServerException::MyServerException(std::string msg) :
        runtime_error(msg) {
}

void TcpServer::shut_down() {
    close_notify();
}

size_t TcpServer::getCountClients() {
    return clients.size();
}
