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
string GameServer::create_json_array(InputIterator beg, InputIterator end) {
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
    last_step{-1, -1}
{
    cnt_fig_for_win = 3;
    if (size > 4) {
        cnt_fig_for_win = 5;
    }
    field.resize(size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            field[i].push_back(UNDEFINED);
        }
    }
}

bool GameServer::GameField::is_good(int row, int column) {
    return row > -1 && column > -1 &&
           row < size && column < size;
}

int GameServer::GameField::get_num_fig(int row, int column, int dx, int dy, char fig) {
    int result = 0;
    while(true) {
        row += dx;
        column += dy;
        if (!is_good(row, column) || field[row][column] != fig) {
            break;
        }
        result++;
    }
    return result;
}

void GameServer::GameField::set_fig(int row, int column, char fig) {
    if (!is_good(row, column))
        throw invalid_argument("\n row=" + to_string(row) + 
                               "\n column=" + to_string(column) + 
                               "\n where size=" + to_string(size));
    last_step = {row, column};
    field[row][column] = fig;
    int cntDirecions = sizeof(dx) / sizeof(int);

    for (int curDir = 0; curDir < cntDirecions; ++curDir) { 
        int numFigs = get_num_fig(row, column, dx[curDir], dy[curDir], fig);
        numFigs += get_num_fig(row, column, -dx[curDir], -dy[curDir], fig);
        if (1 + numFigs == cnt_fig_for_win) {
            winner = fig;
            break;
        }
    }
}

int GameServer::GameField::get_size() {
    return size;
}

vector<vector<char>>& GameServer::GameField::get_field() {
    return field;
}


char GameServer::GameField::get_winner() {
    return winner;
}

pair<int, int> GameServer::GameField::get_last_step() {
    return last_step;
}

void GameServer::GameField::restart() {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            field[i][j] = UNDEFINED;
        }
    }
    winner = UNDEFINED;
    last_step = {-1, -1};
}

void GameServer::Player::set_fig(int row, int column) {
    game_field->set_fig(row, column, my_fig);
}

GameServer::Player::Player(string name) :
    name(name),
    my_fig(GameServer::GameField::FIG_O),
    creator(false),
    my_turn(false),
    game_field(nullptr) {}


int GameServer::add_player(string new_name) {
    if (used_names.count(new_name) != 0) return 400;

    used_names.insert(new_name);
    get_player[new_name] = unique_ptr<Player>(new Player(new_name));
    return 200;
}

void GameServer::delete_player(string name, string partner_name) {
    Player* player = get_player[name].get();

    ready_players[partner_name].erase(name);

    ready_players.erase(name);
    creators_in_waiting.erase(name);

    used_names.erase(name);
    if (player != nullptr) {
        play_pairs.erase(play_pairs[player]);
        play_pairs.erase(player);

        get_player[name].reset();
        get_player.erase(name);
    }
}

void GameServer::set_creator(string name, int field_size) {
    Player* player = get_player[name].get();
    if (player == nullptr) return;
    player->creator = true;
    player->my_turn = true;
    player->game_field = shared_ptr<GameField>(new GameField(field_size));
    player->my_fig = 'X';
    creators_in_waiting.insert(name);
}

void GameServer::cancel_creator(string name) {
    Player* player = get_player[name].get();
    if (player == nullptr) return;
    player->creator = false;
    player->my_turn = false;
    player->game_field.reset();
    creators_in_waiting.erase(name);
}

// player join with creator. Player waiting begin of the game
int GameServer::join_with(string player, string creator) {
    if (creators_in_waiting.count(creator) == 0)
        return 400; 
    ready_players[creator].insert(player);
    return 200;
}

// creator accept invite. Game start 
int GameServer::start_play(string player, string creator) {
    if (creators_in_waiting.count(creator) == 0)
        return 400;
    creators_in_waiting.erase(creator);
    ready_players.erase(creator);
    Player* creatorPlayer = get_player[creator].get();
    Player* partnerPlayer = get_player[player].get();
    play_pairs[creatorPlayer] = partnerPlayer;
    play_pairs[partnerPlayer] = creatorPlayer;

    partnerPlayer->game_field = creatorPlayer->game_field;
    return 200;
}

bool GameServer::check_if_accept(string name){
    Player* player = get_player[name].get();
    return play_pairs.count(player);
}

string GameServer::get_all_players() {
    return create_json_array(used_names.begin(), used_names.end());
}

string GameServer::get_creators() {
    auto beg = creators_in_waiting.begin();
    auto end = creators_in_waiting.end();
    string message = create_json_array(beg, end);
    return message;
}

string GameServer::get_ready_players(string creator) {
    auto beg = ready_players[creator].begin();
    auto end = ready_players[creator].end();
    return create_json_array(beg, end);
}

string GameServer::get_field(string name) {
    if (get_player.count(name) == 0) throw invalid_argument("unknown name");
    Player* player = get_player[name].get();
    if (player->game_field == nullptr) throw invalid_argument("unknown name");
    
    vector<vector<char>>& field = player->game_field->get_field();
    string result = "[";
    for (int i = 0; i < (int) field.size(); ++i) {
       result += create_json_array(field[i].begin(), field[i].end()) + ",";
    }
    if (result != "[")
        result.pop_back();
    result += "]";
    return result;
}

void GameServer::restart_game(std::string name) {
    Player* player = get_player[name].get();
    if (player->game_field->get_winner() == GameField::UNDEFINED) return;
    int cnt_wins = player->num_wins + play_pairs[player]->num_wins;
    player->my_turn = true ^ cnt_wins % 2;
    play_pairs[player]->my_turn = false ^ cnt_wins % 2;
    if (!player->creator) {
        swap(player->my_turn, play_pairs[player]->my_turn);
    } 
    player->game_field->restart();
}

string GameServer::get_game_state_json(string name) {
    string result;
    if (get_player.count(name) == 0) return "";
    Player* player = get_player[name].get();
    
    result = "{"
             "\"allPlayers\": " + get_all_players() + ",";
    
    if (play_pairs.count(player) == 1) {
       GameField* gameField = player->game_field.get();

       result += "\"myTurn\": " + boolToString(player->my_turn) + ","
                 "\"fieldSize\": " + to_string(player->game_field->get_size()) + ","
                 "\"myFig\": \"" + player->my_fig + "\","
                 "\"numWins\": " + to_string(player->num_wins) + ",";

       if (gameField->get_winner() != GameField::UNDEFINED) {
           string iWin = gameField->get_winner() == player->my_fig ? "true" : "false";
           result += "\"win\": " + iWin + ",";
       }

       if (player->my_turn) {
           pair<int, int> lastStep = gameField->get_last_step();
           if (lastStep.first != -1) {
               int row = lastStep.first;
               int column = lastStep.second;
               char fig = gameField->get_field()[row][column];
               result += "\"lastStep\": {"
                         "\"row\": " + to_string(row) + ","
                         "\"column\": " + to_string(column) + ","
                         "\"fig\": \"" + fig + "\"},";
            }
        }
    } else {
        if (player->creator) {
                result += "\"readyPlayers\": " + get_ready_players(name) + ",";
        } else {
            result += "\"creators\": " + get_creators() + ",";
        }
    }    
    result.pop_back();
    result += "}";
    
    return result;
}

void GameServer::put_fig(string name, int row, int column) {
    Player* player = get_player[name].get();
    player->set_fig(row, column);
    player->my_turn = false;
    if (!play_pairs.count(player)) {
        return;
    }
    play_pairs[player]->my_turn = true;

    if (player->game_field->get_winner() != GameServer::GameField::UNDEFINED) {
        if (player->game_field->get_winner() == player->my_fig) {
            player->num_wins++;
        } else {
            play_pairs[player]->num_wins++;
        }
    }
}
