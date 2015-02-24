#include "TcpSocket.h"
#include "TcpServer.h"
#include <unistd.h>

void network::TcpSocket::safely_close_socket(const std::string &from_fun, bool enabled_notify_all) {
    if (!is_valid()) return;
    if (enabled_notify_all)
        notify_all();

    set_valid(false);
    if (::close(get_sfd()) == -1) {
        perror((from_fun + ": safely_close_socket").c_str());
        throw MyServerException("Something go wrong. See above.");
    }
}
