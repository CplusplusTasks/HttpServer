#include <fstream>
#include <algorithm>
#include <iostream>
#include "EpollLoop.h"
#include "Server.h"
#include <unordered_map>
#include "HttpResponse.h"
#include "HttpRequest.h"
#define PORT "7777"

using namespace std;
using namespace MyServer;
using namespace MyHttpManager;

const std::string PREFIX = "build/";

string getFile(string path) {
    ifstream in(path);
    string page = "";
    string line;
    while (getline(in, line)) {
        page += line + "\n";
    }
    in.close();
    return page;
}

int main() {
    EpollLoop epoll;
    try {
        Server server(&epoll, "", PORT);
        server.set_callback_on_accept([&server] (Client* client) {
            cerr << "new client: " << client->get_sfd() << endl;
        });

        server.set_callback_on_receive([&server] (Client* client) {
            HttpRequest request(client);

            string path = PREFIX + "demo" + request.getResourcePath();
            string extension = request.getExtensionOfResource();
            if (request.getResourcePath() == "/") {
                path += "index.html";
                extension = "html";
            }
            cerr << "client_sfd" << client->get_sfd() << endl;
            //request.print();

            cerr << "PATH: " << request.getResourcePath() << endl;
            if (request.getResourcePath() == "/response.php") {
                HttpResponse response;
                response.setMessage("<p>queryUrl: " + request.getQueryUrl() + "</p>" +
                "<p>PostMessage: " + request.getPostMessage() + "</p>").send(client);
            } else {
                HttpResponse response;
                response.setExtension(extension).setMessage(getFile(path)).send(client);
            } 
        });

        server.set_callback_on_close([&server] (Client* client) {
            cerr << "close: " << client->get_sfd() << endl;
            //if (server.getCountClients() == 1) {
                //server.shut_down();
            //}
        });

        epoll.start();

    } catch(MyServerException& e) {
        cerr << e.what() << endl;
    }
    return 0;
}
