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
#define PORT "7777"

using namespace std;
using namespace MyServer;
using namespace MyHttpManager;

const string PREFIX = "build/";

struct GameField {
public:
    const int UNDEFINED = -1;
    const int FIG_X = 0;
    const int FIG_O = 1;

    GameField(size_t size) :
        size(size)
    {
        field.resize(size);
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                field[i].push_back(UNDEFINED);
            }
        }
    }

    bool isGood(size_t row, size_t column) {
        return row < size && column < size;
    }

    void setFig(size_t row, size_t column, int fig) {
        if (!isGood(row, column)) 
            throw std::invalid_argument("\n row=" + to_string(row) + 
                                        "\n column=" + to_string(column) + 
                                        "\n where size=" + to_string(size));
        field[row][column] = fig;
    }

    ssize_t getSize() {
        return size;
    }
private:
    size_t size;
    vector< vector<short> > field;
};

struct Player {
public:
    bool creator;
    bool myTurn;
    shared_ptr<GameField> gameField;
};
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
    set<string> usedNames;
    unordered_set<string> creatorsInWaiting;
    unordered_map< string, unique_ptr<Player> > getPlayer;
    unordered_map<Player*, Player*> playPairs;
    unordered_map< string, vector<string> > getReadyPlayers;
    try {
        Server server(&epoll, "172.16.22.197", PORT);
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
            //cerr << "PATH: " << resPath << endl;

            string message;
            HttpResponse response;
            if (resPath == "/new_player") {
                string newPlayer = request.getPostMapQuery().begin()->second;
                cerr << "newPlayer: " << newPlayer << endl;
                cerr << "sfd: " << client->get_sfd() << endl;
                if (usedNames.count(newPlayer) == 0) {
                    usedNames.insert(newPlayer);
                    getPlayer[newPlayer] = unique_ptr<Player>(new Player);
                    response.setStatusCode(200); // Bad Request
                } else {
                    response.setStatusCode(400); // Bad Request
                }
            } else if (resPath == "/get_all_players") {
                for (auto curPlayer = usedNames.begin(); curPlayer != usedNames.end(); curPlayer++) {
                    message += "<li>" + *curPlayer + "</li>";
                }
            } else if (resPath == "/leave_game") {
                string name = request.getPostMapQuery().begin()->second;
                cerr << "LEAVE: " + name << endl;
                usedNames.erase(name);
                playPairs.erase(getPlayer[name].get());
                getPlayer.erase(name);
                creatorsInWaiting.erase(name);
            } else if (resPath == "/create_field") {
                auto queries = request.getPostMapQuery();
                ssize_t fieldSize = stoi(queries["size"]);
                string name = queries["name"];
                Player* player = getPlayer[name].get();
                player->creator = true;
                player->myTurn = true;
                player->gameField = shared_ptr<GameField>(new GameField(fieldSize));
                creatorsInWaiting.insert(name);

                cerr << "creator is: " << name << endl;
            } else if (resPath == "/get_creators") {
                for (auto curCreator = creatorsInWaiting.begin(); 
                     curCreator != creatorsInWaiting.end(); curCreator++) 
                {
                    message += "<tr><td>" + *curCreator + "</td></tr>";
                }
            } else if (resPath == "/join_with") {
                auto queries = request.getPostMapQuery();
                string with = queries["with"];
                string name = queries["name"];
                
                if (creatorsInWaiting.count(with)) {
                    getReadyPlayers[with].push_back(name);
                } else {
                    response.setStatusCode(400);
                }
                
                //if (creatorsInWaiting.count(with)) {
                    //creatorsInWaiting.erase(with);
                    //Player* creator = getPlayer[with].get();
                    //Player* current = getPlayer[name].get();
                    //playPairs[creator] = current;
                    //playPairs[current] = creator;

                    //current->gameField = creator->gameField;
                    //message = "size=" + to_string(current->gameField->getSize());
                //} else {
                    //response.setStatusCode(400);
                //}
            } else if (resPath == "/get_ready_players") {
                auto queries = request.getPostMapQuery();
                string name = queries["name"];
                auto beg = getReadyPlayers[name].begin();
                auto end = getReadyPlayers[name].end();
                for (; beg != end; beg++) {
                    message += "<tr><td>" + *beg + "</td></tr>";
                }
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
