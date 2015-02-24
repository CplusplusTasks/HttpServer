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

    int add_player(std::string);
    void delete_player(std::string name, std::string partner_name);
    void set_creator(std::string name, int field_size);
    void cancel_creator(std::string name);

    // player join with creator. Player waiting begin of the game
    int join_with(std::string player, std::string creator);

    // creator accept invite. Game start.
    int start_play(std::string player, std::string creator);

    // check if creator accept invite from player. return not empty string if true
    bool check_if_accept(std::string name);

    std::string get_all_players();
    std::string get_creators();
    std::string get_ready_players(std::string creator);
    std::string get_field(std::string name);
    bool get_turn(std::string name);
    std::string get_game_state_json(std::string name);

    void put_fig(std::string name, int row, int column);
    void restart_game(std::string name);
private:
    template<typename InputIterator>
    std::string create_json_array(InputIterator begin, InputIterator end);

    std::set<std::string> used_names;
    std::unordered_set<std::string> creators_in_waiting;
    std::unordered_map< std::string, std::unique_ptr<Player> > get_player;
    std::unordered_map<Player*, Player*> play_pairs;
    std::unordered_map< std::string, std::set<std::string> > ready_players;
};



struct GameServer::GameField {
public:
    const static char UNDEFINED = ' ';
    const static char FIG_X = 'X';
    const static char FIG_O = 'O';

    GameField(int size);
    void set_fig(int row, int column, char fig);
    char get_winner(); // one of 3 possible variants
    int get_size();
    std::pair<int, int> get_last_step();
    std::vector< std::vector<char> >& get_field();
    void restart();

private:
    bool is_good(int, int);
    int get_num_fig(int row, int column, int dx, int dy, char fig);

    const static int dx[];
    const static int dy[];
    const int size;
    int cnt_fig_for_win;
    char winner;
    std::pair<int, int> last_step;
    std::vector< std::vector<char> > field;
};

struct GameServer::Player {
public:
    Player(std::string name);
    std::string name;
    int num_wins;
    char my_fig;
    bool creator;
    bool my_turn;
    std::shared_ptr<GameField> game_field;
    void set_fig(int row, int column);
};
#endif
