//
// Created by tomek on 20.05.2022.
//

#ifndef BOMBOWEROBOTY_KLIENT_TYPES_H
#define BOMBOWEROBOTY_KLIENT_TYPES_H
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <unordered_set>

using std::string;
using std::vector;
using std::map;

using port_t = uint16_t;
using ip_t = string;

using address_t = std::tuple<ip_t, port_t>;

using list_and_map_size_t = uint32_t;
using str_size_t = uint8_t;
using message_nr_t = uint8_t;

using turn_number_t = uint16_t;
using game_len_t = uint16_t;
using explosion_radius_t = uint16_t;

using coordinates_t = uint16_t;
using signed_coordinates_t = int32_t;
struct position_t {
    coordinates_t x,y;

//    position_t(coordinates_t x, coordinates_t y) : x(x), y(y) {}

    bool operator==(const position_t& other) const{
        return this->x == other.x && this->y == other.y;
    }

    struct HashFunction{
        size_t operator()(const position_t& point) const{
            size_t xHash = std::hash<coordinates_t>()(point.x);
            size_t yHash = std::hash<coordinates_t>()(point.y) << 1;
            return xHash ^ yHash;
        }
    };
};
using positions_list = vector<position_t>;
using positions_set = std::unordered_set<position_t, position_t::HashFunction>;

using bomb_timer_t = uint16_t;
using bombid_t = uint32_t;
struct bomb_t {
    position_t position;
    bomb_timer_t timer;
};
using bombs_map_t = std::map<bombid_t, bomb_t>;

using playerid_t = uint8_t;
using players_count_t = uint8_t;
struct player_t {
    string name, address;
};
using players_id_list = vector<playerid_t>;
using players_id_set = std::unordered_set<playerid_t>;
using players_t = map<playerid_t, player_t>;
using players_positions_t = map<playerid_t, position_t>;

using score_t = uint32_t;
using scores_t = map<playerid_t, score_t>;

enum ServerMessage {Hello, AcceptedPlayer, GameStarted, Turn, GameEnded};
enum ClientMessageToServer {Join, PlaceBomb, PlaceBlock, Move};
enum GuiMessage {Lobby, Game};
enum Direction {Up, Right, Down, Left, Unset};

enum InputMessage_nr {PlaceBomb_in, PlaceBlock_in, Move_in, NotAvailableOrInvalid_in};
struct InputMessage {
    InputMessage_nr type;
    Direction direction;

    InputMessage(InputMessage_nr type) : type(type) {
        direction = Unset;
    }
};

#endif //BOMBOWEROBOTY_KLIENT_TYPES_H
