#include <fstream>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include "EpollLoop.h"
#include "HttpServer.h"
#include "GameServer.h"

using namespace std;
using namespace network;

const string PREFIX_PATH = "www/";

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
    set<int> s;
    EpollLoop epoll;
    GameServer gameServer;
    try {
        string addr, port;
        {
            ifstream in("config.txt");
            in >> addr >> port;
        }
        HttpServer server(&epoll, addr, port);
        server.set_callback_on_accept([&server] (HttpClient* client) {
            cerr << "new: " << client->get_sfd() << endl;
        });

        server.set_callback_on_receive([&server, &gameServer] (HttpClient* client) {
            HttpRequest request(client->get_next_request());

            string message;
            string resPath = request.getResourcePath();
            string resExtension = "html";
            int statusCode = 200;
            auto queries = request.getPostMapQuery();

            if (resPath == "/new_player") {
                statusCode = gameServer.add_player(queries["name"]);
            } else if (resPath == "/leave_game") {
                string name = queries["name"]; 
                string partnerName = queries["with"];
                gameServer.delete_player(name, partnerName);
            } else if (resPath == "/create_field") {
                int fieldSize = stoi(queries["size"]);
                string name = queries["name"];
                gameServer.set_creator(name, fieldSize);
            } else if (resPath == "/join_with") {
                string creator = queries["with"];
                string player = queries["name"];
                statusCode = gameServer.join_with(player, creator);
            } else if (resPath == "/start_play") {
                string creator = queries["name"];
                string player = queries["with"];
                statusCode = gameServer.start_play(player, creator);
            } else if (resPath == "/cancel_create_field") {
                gameServer.cancel_creator(queries["name"]);
            } else if (resPath == "/put_fig") {
                string name = queries["name"];
                int row = stoi(queries["row"]);
                int column = stoi(queries["column"]);
                gameServer.put_fig(name, row, column);
            } else if (resPath == "/play_again") {
                cerr << "again" << endl;
                string name = queries["name"];
                gameServer.restart_game(name);
            } else if (resPath == "/get_game_state") {
                string name = queries["name"];
                message = gameServer.get_game_state_json(name);
                resExtension = "json";
            } else {
                string path = PREFIX_PATH + resPath;
                resExtension = request.getExtensionOfResource();
                if (resExtension.empty()) {
                    resExtension = "html";
                }

                if (resPath == "/") {
                    path += "index.html";
                }
                message = getFile(path);
            } 

            HttpResponse response;
            response
                .setStatusCode(statusCode)
                .setExtension(resExtension)
                .setMessage(message);
            client->send(response);
        });

        server.set_callback_on_close([&] (HttpClient* client) {
            cerr << "close: " << client->get_sfd() << endl;
            // TBD
            // send msg to partner
        });

        epoll.start();

    } catch(MyServerException& e) {
        cerr << e.what() << endl;
    }
    return 0;
}

// /home/eugene/university/programming/practice/c++/questions
