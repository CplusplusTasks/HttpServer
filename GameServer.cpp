#include "GameServer.h"

using namespace std;

const char GameServer::GameField::UNDEFINED;
const char GameServer::GameField::FIG_X;
const char GameServer::GameField::FIG_O;
const int GameServer::GameField::dx[] = {-1, -1, 0, 1,  1,  1,  0, -1};
const int GameServer::GameField::dy[] = { 0,  1, 1, 1,  0, -1, -1, -1};

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
    winner(UNDEFINED)
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

void GameServer::GameField::setFig(int row, int column, char fig) {
    if (!isGood(row, column)) 
        throw std::invalid_argument("\n row=" + to_string(row) + 
                                    "\n column=" + to_string(column) + 
                                    "\n where size=" + to_string(size));
    field[row][column] = fig;
    int cntDirecions = sizeof(dx) / sizeof(int);
    for (int curDirection = 0; curDirection < cntDirecions && winner == UNDEFINED; ++curDirection) { 
        int new_row = row; 
        int new_column = column; 
        winner = fig;
        for (int i = 0; i < cntFigForWin; ++i) {
            new_row += dx[curDirection];
            new_column += dy[curDirection];
            if (!isGood(new_row, new_column)) break;
            if (field[new_row][new_column] != fig) {
                winner = UNDEFINED;
                break;
            }
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

int GameServer::addPlayer(string newName) {
    //cerr << "newName: " << newName << endl;
    //cerr << "sfd: " << client->get_sfd() << endl;
    if (usedNames.count(newName) != 0) return 400;

    usedNames.insert(newName);
    getPlayer[newName] = unique_ptr<Player>(new Player);
    return 200;
}

void GameServer::Player::setFig(int row, int column) {
    gameField->setFig(row, column, myFig); 
}

GameServer::Player::Player() :
    myFig(GameServer::GameField::FIG_O),
    creator(false),
    myTurn(false),
    gameField(nullptr) {}

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

using namespace std;
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

//check if creator accept invite from player
string GameServer::checkIfAccept(std::string name){
    Player* player = getPlayer[name].get();
    if (playPairs.count(player) == 0) 
        return "";
    int size = player->gameField->getSize();
    string message="size=" + to_string(size);
    return message;
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
    Player* player = getPlayer[name].get();
    if (player == nullptr || player->gameField == nullptr) return "error";
    
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

void GameServer::putFig(std::string name, int row, int column) {
    Player* player = getPlayer[name].get();
    player->setFig(row, column);
    player->myTurn = false;
    playPairs[player]->myTurn = true;
}

bool GameServer::getTurn(string name) {
    return getPlayer[name]->myTurn;
}
