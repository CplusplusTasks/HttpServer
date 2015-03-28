#include "TcpClient.h"

#define is_closed if_status(0)
#define end_of_data (if_status(-1) && errno == EAGAIN)
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

}

TcpClient::TcpClient(int sfd, const string &ip, EpollLoop *epoll, TcpServer *parent) :
        sfd(sfd),
        mode(EDGE_MODE),
        valid(true),
        ip(ip),
        epoll(epoll),
        parent(parent),
        callback_on_receive(NULL),
        callback_on_close(NULL)
{}

string TcpClient::get_ip() const {
    check_on_valid
    return ip;
}

void TcpClient::send(const std::string &msg) const {
    check_on_valid
    epoll->add_to_send(sfd, msg);
}

void TcpClient::close_notify() {
    safely_close_socket(cur_loc, true);
}

void TcpClient::close() {
    safely_close_socket(cur_loc, false);
    epoll->close_socket(sfd);
    parent->remove_client(sfd);
}

bool TcpClient::is_valid() const {
    return valid;
}

int TcpClient::get_sfd() const {
    return sfd;
}

void TcpClient::set_valid(bool valid) {
    this->valid = valid;
}

// -1 - data exhausted or error occurred
// 0 - socket has closed
// 1 - success
ssize_t TcpClient::read_bytes(char *buff, size_t num) {
    check_on_valid
    ssize_t receiving_bytes = recv(sfd, buff, num, 0);
    if (receiving_bytes == -1) {
        if (errno != EAGAIN) { // read all data
            perror(cur_loc.c_str());
            safely_close_socket(cur_loc);
        }
    } else if (receiving_bytes == 0) {
        cerr << "Connection is closed. Client ip: " << ip << ". Client sfd: " << sfd << endl;
        safely_close_socket(cur_loc);
    }
    return receiving_bytes;
}

// -1 - data exhausted or error occurred
// 0 - socket has closed
// 1 - success
// INV: buff is empty

int TcpClient::read_all(vector<char>& buff) {
    check_on_valid
    if (!valid) return -1;
    unsigned long cnt_bytes = 1000;
    buff.clear();
    int size_before = (int) buff.size();
    for (int i = 0; ; i++) {
        buff.resize(size_before + cnt_bytes * (i + 1));
        ssize_t receiving_bytes = recv(sfd, buff.data() + size_before + i * cnt_bytes, cnt_bytes, 0);
        if (receiving_bytes == -1) {
            buff.resize(size_before + cnt_bytes * i);
            if (errno != EAGAIN) { // read all data
                perror(cur_loc.c_str());
                safely_close_socket(cur_loc);
                return -1;
            }
            break;
        } else if (receiving_bytes == 0) {
            buff.resize(size_before + cnt_bytes * i + receiving_bytes);
            cerr << "Connection is closed. Client ip: " << ip << ". Client sfd: " << sfd << endl;
            safely_close_socket(cur_loc);
            return 0;
        }
        if (cnt_bytes * (i + 1) != cnt_bytes * i + receiving_bytes) {
            buff.resize(size_before + cnt_bytes * i + receiving_bytes);
            if (end_of_data) {
                break;
            }
        }
    }
    return 1;
}

void TcpClient::on_receive() {
    if (is_closed) {
        notify_all();
    } else if (callback_on_receive != NULL) {
        callback_on_receive();
        if (!valid) return;

        if (mode == EDGE_MODE) {
            unsigned long n = 100;
            vector<char> temp_buff(n);
            while (read_bytes(temp_buff.data(), n - 1) > 0);
        }
    }
}

bool TcpClient::if_status(int value) const {
    vector<char> temp_buff(1);
    ssize_t status = recv(sfd, temp_buff.data(), 1, MSG_PEEK);
    return status == value;
}

void TcpClient::set_callback_on_receive(function<void()> callback_on_receive) {
    this->callback_on_receive = callback_on_receive;
}

void TcpClient::set_callback_on_close(function<void()> callback_on_close) {
    this->callback_on_close = callback_on_close;
}

void TcpClient::notify_all() {
    if (callback_on_close != NULL) {
        callback_on_close();
    }
    if (!valid) return;
    epoll->close_socket(sfd);
    parent->remove_client(sfd);
}

TcpClient::TcpClient(TcpClient &&client) {
    this->sfd = client.sfd;
    this->mode = client.mode;
    this->valid = client.valid;
    this->ip = client.ip;
    this->epoll = client.epoll;
    this->parent = client.parent;
    client.valid = false;
}
