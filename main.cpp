#include <fstream>
#include <algorithm>
#include <iostream>
#include "EpollLoop.h"
#include "Server.h"
#include <unordered_map>
#include "HttpResponse.h"
#include "HttpRequest.h"
#include <set>
#define PORT "7777"

using namespace std;
using namespace MyServer;
using namespace MyHttpManager;

const string PREFIX = "build/";

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
    std::set<string> players;
    try {
        Server server(&epoll, "10.0.0.12", PORT);
        server.set_callback_on_accept([&server] (Client* client) {
            //cerr << "new client: " << client->get_sfd() << endl;
        });

        server.set_callback_on_receive([&] (Client* client) {
            HttpRequest request(client);

            string resPath = request.getResourcePath();
            string path = PREFIX + "demo" + resPath;
            string extension = request.getExtensionOfResource();
            if (extension.empty()) {
                extension = "html";
            }

            if (resPath == "/") {
                path += "index.html";
            }
            //cerr << "client_sfd" << client->get_sfd() << endl;
            //request.print();
            cerr << "PATH: " << resPath << endl;

            string message;
            HttpResponse response;
            if (resPath == "/new_player") {
                string newPlayer = request.getPostMapQuery().begin()->second;
                cerr << "newPlayer: " << newPlayer << endl;
                cerr << "sfd: " << client->get_sfd() << endl;
                if (players.count(newPlayer) == 0) {
                    players.insert(newPlayer);
                    response.setStatusCode(200); // Bad Request
                } else {
                    response.setStatusCode(400); // Bad Request
                }
            } else if (resPath == "/get_players") {
                for (auto curPlayer = players.begin(); curPlayer != players.end(); curPlayer++) {
                    message += "<li>" + *curPlayer + "</li>";
                }
            } else if (resPath == "/leave_game"){
                string name = request.getPostMapQuery().begin()->second;
                cerr << "LEAVE: " + name << endl;
                players.erase(name);
            } else {
                message = getFile(path);
            } 

            response.setExtension(extension).setMessage(message).send(client);
        });

        server.set_callback_on_close([&] (Client* client) {
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
