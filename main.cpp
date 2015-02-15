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
        Server server(&epoll, "10.0.0.12", PORT);
        server.set_callback_on_accept([&server] (Client*) {
            //cerr << "new: " << client->get_sfd() << endl;
        });

        server.set_callback_on_receive([&server, &gameServer] (Client* client) {
            HttpRequest request(client);

            string message;
            string resPath = request.getResourcePath();
            string resExtension = "html";
            int statusCode = 200;
            auto queries = request.getPostMapQuery();

            if (resPath == "/new_player") {
                statusCode = gameServer.addPlayer(queries["name"]);
            } else if (resPath == "/leave_game") {
                string name = queries["name"]; 
                string partnerName = queries["with"]; 
                gameServer.deletePlayer(name, partnerName);
                cerr << "LEAVE: " << name << endl;
            } else if (resPath == "/create_field") {
                int fieldSize = stoi(queries["size"]);
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
                int row = stoi(queries["row"]);
                int column = stoi(queries["column"]);
                gameServer.putFig(name, row, column);
            } else if (resPath == "/play_again") {
                cerr << "again" << endl;
                string name = queries["name"];
                gameServer.restartGame(name); 
            } else if (resPath == "/get_game_state") {
                string name = queries["name"];
                message = gameServer.getGameStateJson(name);
                resExtension = "json";
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

        server.set_callback_on_close([&] (Client*) {
            //cerr << "close: " << client->get_sfd() << endl;
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
