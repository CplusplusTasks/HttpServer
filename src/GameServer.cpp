#include "GameServer.h"
#include <iostream>
#include <cassert>

using namespace std;

const char GameServer::GameField::UNDEFINED;
const char GameServer::GameField::FIG_X;
const char GameServer::GameField::FIG_O;
const int GameServer::GameField::dx[] = {-1, -1, 0,  1};
const int GameServer::GameField::dy[] = { 0,  1, 1, 1};

static string boolToString(bool b) {
    return b ? "true" : "false";
}

template<typename InputIterator>
string GameServer::createJsonArray(InputIterator beg, InputIterator end) {
    string message = "[";
    for (; beg != end; beg++) {
        message = message + "\"" + *beg + "\",";
    }

    if (message != "[") 
        message.pop_back();
    message += "]";
    return message;
}

GameServer::GameField::GameField(int size) :
    size(size),
    winner(UNDEFINED),
    lastStep{-1, -1}
{
    cntFigForWin = 3;
    if (size > 4) {
        cntFigForWin = 5;
    }
    field.resize(size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            field[i].push_back(UNDEFINED);
        }
    }
}

bool GameServer::GameField::isGood(int row, int column) {
    return row > -1 && column > -1 &&
           row < size && column < size;
}

int GameServer::GameField::getNumFig(int row, int column, int dx, int dy, char fig) {
    int result = 0;
    while(true) {
        row += dx;
        column += dy;
        if (!isGood(row, column) || field[row][column] != fig) {
            break;
        }
        result++;
    }
    return result;
}

void GameServer::GameField::setFig(int row, int column, char fig) {
    if (!isGood(row, column)) 
        throw invalid_argument("\n row=" + to_string(row) + 
                               "\n column=" + to_string(column) + 
                               "\n where size=" + to_string(size));
    lastStep = {row, column};
    field[row][column] = fig;
    int cntDirecions = sizeof(dx) / sizeof(int);

    for (int curDir = 0; curDir < cntDirecions; ++curDir) { 
        int numFigs = getNumFig(row, column, dx[curDir], dy[curDir], fig);
        numFigs += getNumFig(row, column, -dx[curDir], -dy[curDir], fig);
        if (1 + numFigs == cntFigForWin) {
            winner = fig;
            break;
        }
    }
}

int GameServer::GameField::getSize() {
    return size;
}

vector<vector<char>>& GameServer::GameField::getField() {
    return field;
}


char GameServer::GameField::getWinner() {
    return winner;
}

pair<int, int> GameServer::GameField::getLastStep() {
    return lastStep;
}

void GameServer::GameField::restart() {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            field[i][j] = UNDEFINED;
        }
    }
    winner = UNDEFINED;
    lastStep = {-1, -1};
}

void GameServer::Player::setFig(int row, int column) {
    gameField->setFig(row, column, myFig); 
}

GameServer::Player::Player(string name) :
    name(name),
    myFig(GameServer::GameField::FIG_O),
    creator(false),
    myTurn(false),
    gameField(nullptr) {}


int GameServer::addPlayer(string newName) {
    if (usedNames.count(newName) != 0) return 400;

    usedNames.insert(newName);
    getPlayer[newName] = unique_ptr<Player>(new Player(newName));
    return 200;
}

void GameServer::deletePlayer(string name, string partnerName) {
    Player* player = getPlayer[name].get();

    readyPlayers[partnerName].erase(name);

    readyPlayers.erase(name);
    creatorsInWaiting.erase(name);

    usedNames.erase(name);
    if (player != nullptr) {
        playPairs.erase(playPairs[player]);
        playPairs.erase(player);

        getPlayer[name].reset();
        getPlayer.erase(name);
    }
}

void GameServer::setCreator(string name, int fieldSize) {
    Player* player = getPlayer[name].get();
    if (player == nullptr) return;
    player->creator = true;
    player->myTurn = true;
    player->gameField = shared_ptr<GameField>(new GameField(fieldSize));
    player->myFig = 'X';
    creatorsInWaiting.insert(name);
}

void GameServer::cancelCreator(string name) {
    Player* player = getPlayer[name].get();
    if (player == nullptr) return;
    player->creator = false;
    player->myTurn = false;
    player->gameField.reset();
    creatorsInWaiting.erase(name);
}

// player join with creator. Player waiting begin of the game
int GameServer::joinWith(string player, string creator) { 
    if (creatorsInWaiting.count(creator) == 0) 
        return 400; 
    readyPlayers[creator].insert(player);
    return 200;
}

// creator accept invite. Game start 
int GameServer::startPlay(string player, string creator) {
    if (creatorsInWaiting.count(creator) == 0) 
        return 400;
    creatorsInWaiting.erase(creator);
    readyPlayers.erase(creator);
    Player* creatorPlayer = getPlayer[creator].get();
    Player* partnerPlayer = getPlayer[player].get();
    playPairs[creatorPlayer] = partnerPlayer;
    playPairs[partnerPlayer] = creatorPlayer;

    partnerPlayer->gameField = creatorPlayer->gameField;
    return 200;
}

bool GameServer::checkIfAccept(string name){
    Player* player = getPlayer[name].get();
    return playPairs.count(player);
}

string GameServer::getAllPlayers() {
    return createJsonArray(usedNames.begin(), usedNames.end());
}

string GameServer::getCreators() {
    auto beg = creatorsInWaiting.begin();
    auto end = creatorsInWaiting.end();
    string message = createJsonArray(beg, end);
    return message;
}

string GameServer::getReadyPlayers(string creator) {
    auto beg = readyPlayers[creator].begin();
    auto end = readyPlayers[creator].end();
    return createJsonArray(beg, end);
}

string GameServer::getField(string name) {
    if (getPlayer.count(name) == 0) throw invalid_argument("unknown name");
    Player* player = getPlayer[name].get();
    if (player->gameField == nullptr) throw invalid_argument("unknown name");
    
    vector<vector<char>>& field = player->gameField->getField();
    string result = "[";
    for (int i = 0; i < (int) field.size(); ++i) {
       result += createJsonArray(field[i].begin(), field[i].end()) + ","; 
    }
    if (result != "[")
        result.pop_back();
    result += "]";
    return result;
}

void GameServer::restartGame(std::string name) {
    Player* player = getPlayer[name].get();
    if (player->gameField->getWinner() == GameField::UNDEFINED) return;
    int cntWins = player->numWins + playPairs[player]->numWins;
    player->myTurn = true ^ cntWins % 2;
    playPairs[player]->myTurn = false ^ cntWins % 2;
    if (!player->creator) {
        swap(player->myTurn, playPairs[player]->myTurn);
    } 
    player->gameField->restart();
}

string GameServer::getGameStateJson(string name) {
    string result;
    if (getPlayer.count(name) == 0) return "";
    Player* player = getPlayer[name].get();
    
    result = "{"
             "\"allPlayers\": " + getAllPlayers() + ",";
    
    if (playPairs.count(player) == 1) {
       GameField* gameField = player->gameField.get();

       result += "\"myTurn\": " + boolToString(player->myTurn) + ","
                 "\"fieldSize\": " + to_string(player->gameField->getSize()) + ","
                 "\"myFig\": \"" + player->myFig + "\","
                 "\"numWins\": " + to_string(player->numWins) + ",";

       if (gameField->getWinner() != GameField::UNDEFINED) {
           string iWin = gameField->getWinner() == player->myFig ? "true" : "false";
           result += "\"win\": " + iWin + ",";
       }

       if (player->myTurn) {
           pair<int, int> lastStep = gameField->getLastStep(); 
           if (lastStep.first != -1) {
               int row = lastStep.first;
               int column = lastStep.second;
               char fig = gameField->getField()[row][column];
               result += "\"lastStep\": {"
                         "\"row\": " + to_string(row) + ","
                         "\"column\": " + to_string(column) + ","
                         "\"fig\": \"" + fig + "\"},";
            }
        }
    } else {
        if (player->creator) {
                result += "\"readyPlayers\": " + getReadyPlayers(name) + ",";
        } else {
            result += "\"creators\": " + getCreators() + ",";
        }
    }    
    result.pop_back();
    result += "}";
    
    return result;
}

void GameServer::putFig(string name, int row, int column) {
    Player* player = getPlayer[name].get();
    player->setFig(row, column);
    player->myTurn = false;
    playPairs[player]->myTurn = true;

    if (player->gameField->getWinner() != GameServer::GameField::UNDEFINED) {
        if (player->gameField->getWinner() == player->myFig) {
            player->numWins++;         
        } else {
            playPairs[player]->numWins++;
        }
    }
}
