#ifndef GAME_SERVER_H
#define GAME_SERVER_H
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <memory>
#include <vector>
#include "HttpRequest.h"
struct GameServer {
public:
    struct Player;
    struct GameField;

    int addPlayer(std::string);
    void deletePlayer(std::string name, std::string partnerName);
    void setCreator(std::string name, int fieldSize);
    void cancelCreator(std::string name);

    // player join with creator. Player waiting begin of the game
    int joinWith(std::string player, std::string creator); 

    // creator accept invite. Game start.
    int startPlay(std::string player, std::string creator);

    // check if creator accept invite from player. return not empty string if true
    bool checkIfAccept(std::string name); 

    std::string getAllPlayers();
    std::string getCreators();
    std::string getReadyPlayers(std::string creator);
    std::string getField(std::string name);
    bool getTurn(std::string name);
    std::string getGameStateJson(std::string name);

    void putFig(std::string name, int row, int column);
    void restartGame(std::string name);
private:
    template<typename InputIterator>
    std::string createJsonArray(InputIterator begin, InputIterator end);

    std::set<std::string> usedNames;
    std::unordered_set<std::string> creatorsInWaiting;
    std::unordered_map< std::string, std::unique_ptr<Player> > getPlayer;
    std::unordered_map<Player*, Player*> playPairs;
    std::unordered_map< std::string, std::set<std::string> > readyPlayers;
};



struct GameServer::GameField {
public:
    const static char UNDEFINED = ' ';
    const static char FIG_X = 'X';
    const static char FIG_O = 'O';

    GameField(int size);
    void setFig(int row, int column, char fig); 
    char getWinner(); // one of 3 possible variants
    int getSize(); 
    std::pair<int, int> getLastStep();
    std::vector< std::vector<char> >& getField();
    void restart();

private:
    bool isGood(int, int); 
    int getNumFig(int row, int column, int dx, int dy, char fig);

    const static int dx[];
    const static int dy[];
    const int size;
    int cntFigForWin;
    char winner;
    std::pair<int, int> lastStep;
    std::vector< std::vector<char> > field;
};

struct GameServer::Player {
public:
    Player(std::string name);
    std::string name;
    int numWins;
    char myFig;
    bool creator;
    bool myTurn;
    std::shared_ptr<GameField> gameField;
    void setFig(int row, int column);
};
#endif
