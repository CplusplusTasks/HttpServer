#include <fstream>
#include <algorithm>
#include <iostream>
#include "EpollLoop.h"
#include "Server.h"
#include <unordered_map>
#include "HttpResponse.h"
#include "HttpRequest.h"
#include <set>
#include <unordered_set>
#include <memory>
#include <iterator>
#include "GameServer.h"
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
    GameServer gameServer;
    try {
        Server server(&epoll, "", PORT);
        server.set_callback_on_accept([&server] (Client* client) {
            cerr << "new: " << client->get_sfd() << endl;
        });

        server.set_callback_on_receive([&server, &gameServer] (Client* client) {
            HttpRequest request(client);

            string message;
            string resPath = request.getResourcePath();
            string resExtension = "html";
            int statusCode = 200;
            auto queries = request.getPostMapQuery();

            if (resPath == "/new_player") {
                gameServer.addPlayer(queries["name"]);
            } else if (resPath == "/leave_game") {
                string name = queries["name"]; 
                string partnerName = queries["with"]; 
                gameServer.deletePlayer(name, partnerName);
                cerr << "LEAVE: " << name << endl;
            } else if (resPath == "/create_field") {
                size_t fieldSize = stoi(queries["size"]);
                string name = queries["name"];
                gameServer.setCreator(name, fieldSize);
            } else if (resPath == "/join_with") {
                string creator = queries["with"];
                string player = queries["name"];
                statusCode = gameServer.joinWith(player, creator); 
            } else if (resPath == "/start_play") {
                string creator = queries["name"];
                string player = queries["with"];
                statusCode = gameServer.startPlay(player, creator);
            } else if (resPath == "/cancel_create_field") {
                 gameServer.cancelCreator(queries["name"]);
            } else if (resPath == "/put_fig") {
                string name = queries["name"];
                size_t row = stoi(queries["row"]);
                size_t column = stoi(queries["column"]);
                gameServer.putFig(name, row, column);
            } else if (resPath == "/check_if_accept") {
                string name = queries["name"];
                message = gameServer.checkIfAccept(name);
                if (message.empty())
                    statusCode = 400;
            } else if (resPath == "/get_creators") {
                message = gameServer.getCreators();
                resExtension = "json";
            } else if (resPath == "/get_all_players") {
                message = gameServer.getAllPlayers();
                resExtension = "json";
            } else if (resPath == "/get_ready_players") {
                message = gameServer.getReadyPlayers(queries["name"]);
                resExtension = "json";
            } else if (resPath == "/get_play_field") {
                string name = queries["name"];
                message = gameServer.getField(name);
                resExtension = "json";
            } else if (resPath == "/get_turn") {
                string name = queries["name"];
                if (!gameServer.getTurn(name)) {
                    statusCode = 400;
                }
            } else {
                string path = PREFIX + "demo" + resPath;
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
                .setMessage(message)
                .send(client);
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
