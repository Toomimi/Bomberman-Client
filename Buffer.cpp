//
// Created by tomek on 21.05.2022.
//

#include <stdexcept>
#include "Buffer.h"

using std::malloc;
using std::free;


void Buffer::check_if_structure_fits_and_write_its_len(size_t s_len, size_t el_size) {
    if (len + (s_len * el_size) >= BUFF_SIZE)
        throw std::length_error("Not enough space in buffer.\n");
    write_number<list_and_map_size_t>((list_and_map_size_t) s_len);
}

Buffer::Buffer() {
    len = 0;
    read_index = 0;
    buff = (char*) malloc(BUFF_SIZE * sizeof(char));
    if (buff == nullptr)
        throw std::bad_alloc();
    memset(buff, UINT8_MAX, BUFF_SIZE * sizeof(char));
}

Buffer::~Buffer() { free(buff);}

void Buffer::clear() {
    read_index = 0;
    len = 0;
    memset(buff, UINT8_MAX, BUFF_SIZE * sizeof(char));
}

void Buffer::clear_read_message() {
    memcpy(buff, buff + read_index, len - read_index);
    len -= read_index;
    read_index = 0;
    memset(buff + len, UINT8_MAX, (BUFF_SIZE - len) * sizeof(char));
}

void Buffer::move_read_index_to_begin() {
    read_index = 0;
}

void Buffer::check_if_enough_data(size_t data_size) const {
    if (read_index + data_size >= len)
        throw std::underflow_error("Not enough data in buffer");
}


char* Buffer::get_buff_end() {
    return buff + len;
}

void Buffer::update_len(size_t written_size) {
    len += written_size;
}

char* Buffer::get_buff() {
    return buff;
}

size_t Buffer::get_len() const {
    return len;
}

void Buffer::write_string(const string& string_to_add) {
    size_t str_size = string_to_add.size();
    if (str_size > UINT8_MAX)
        throw std::length_error("Can't write strings longer than 256\n");

    write_number<str_size_t>((str_size_t)str_size);
    strcpy(buff + len, string_to_add.c_str());
    len += str_size;
}

string Buffer::read_string() {
    size_t str_size = read_number<str_size_t>();
//    check_if_enough_data(str_size);
    string read_str(&buff[read_index], str_size);
    read_index += str_size;
    return read_str;
}

void Buffer::write_position(position_t p) {
//    std::cout << "writing positon: " << p.x << " " << p.y << "\n";
    write_number<uint16_t>(p.x);
    write_number<uint16_t>(p.y);
//    print_buffer("Wrote position");
}

position_t Buffer::read_position() {
    position_t new_position{};
    new_position.x = read_number<coordinates_t>();
    new_position.y = read_number<coordinates_t>();
    return new_position;
}

void Buffer::write_position_list(positions_list& list) {
    check_if_structure_fits_and_write_its_len(list.size(), sizeof(position_t));
    for (position_t p : list)
        write_position(p);
}

positions_list Buffer::read_position_list() {
    auto list_size = read_number<list_and_map_size_t>();
    positions_list new_list;
    for (size_t i = 0; i < list_size; i++)
        new_list.push_back(read_position());
    return new_list;
}

void Buffer::write_position_set(positions_set &set) {
    check_if_structure_fits_and_write_its_len(set.size(), sizeof(position_t));
    for (const auto& position : set)
        write_position(position);
}

positions_set Buffer::read_position_set() {
    auto list_size = read_number<list_and_map_size_t>();
    positions_set new_set;
    for (size_t i = 0; i < list_size; i++)
        new_set.insert(read_position());
    return new_set;
}


void Buffer::write_bomb(bomb_t bomb) {
    write_position(bomb.position);
    write_number<bomb_timer_t>(bomb.timer);
}

bomb_t Buffer::read_bomb() {
    bomb_t new_bomb{};
    new_bomb.position = read_position();
    new_bomb.timer = read_number<bomb_timer_t>();
    return new_bomb;
}

void Buffer::write_bombs_map(bombs_map_t &map) {
    check_if_structure_fits_and_write_its_len(map.size(), sizeof(bomb_t));
    for (auto& b : map)
        write_bomb(b.second);
}

vector<bomb_t> Buffer::read_bombs_list() {
    auto list_size = read_number<list_and_map_size_t>();
    vector<bomb_t> new_list;
    for (size_t i = 0; i < list_size; i++)
        new_list.push_back(read_bomb());
    return new_list;
}

void Buffer::write_player(const player_t &player) {
    write_string(player.name);
    write_string(player.address);
}

player_t Buffer::read_player() {
    player_t new_player;
    new_player.name = read_string();
    new_player.address = read_string();
    return new_player;
}

void Buffer::write_players_id_list(players_id_list &list) {
    check_if_structure_fits_and_write_its_len(list.size(), sizeof(playerid_t));
    for (playerid_t pid : list)
        write_number<playerid_t>(pid);
}

players_id_list Buffer::read_players_id_list() {
    auto list_size = read_number<list_and_map_size_t>();
    players_id_list new_list;
    for (size_t i = 0; i < list_size; i++)
        new_list.push_back(read_number<playerid_t>());
    return new_list;
}

void Buffer::write_players_map(players_t &players) {
//    size_t element_size = sizeof() // string może mieć różne rozmiary, założyć max lub sprawdzać podczas zapisywania czy się mieści
//    check_if_structure_fits_and_write_its_len(players.size(), 0) todo: pisząc serwer pamiętać o niemieszczących się wiadomościach w jednym datagramie
    write_number<list_and_map_size_t>((list_and_map_size_t) players.size());

    for (auto const& el : players) {
        write_number<playerid_t>(el.first);
        write_player(el.second);
    }

}

players_t Buffer::read_players_map() {
    auto map_size = read_number<list_and_map_size_t>();
    players_t new_map;
    for (size_t i = 0; i < map_size; i++) {
        auto key = read_number<playerid_t>();
        player_t val = read_player();
        new_map.insert({key, val});
    }
    return new_map;
}

void Buffer::write_players_positions_map(players_positions_t &players_positions) {
    write_number<list_and_map_size_t>((list_and_map_size_t) players_positions.size());
    for (auto const& el : players_positions) {
        write_number<playerid_t>(el.first);
        write_position(el.second);
    }
}

players_positions_t Buffer::read_players_positions_map() {
    auto map_size = read_number<list_and_map_size_t>();
    players_positions_t new_map;
    for (size_t i = 0; i < map_size; i++) {
        auto key = read_number<playerid_t>();
        position_t val = read_position();
        new_map.insert({key, val});
    }
    return new_map;
}

void Buffer::write_players_scores_map(scores_t &scores) {
    write_number<list_and_map_size_t>((list_and_map_size_t) scores.size());
    for (auto const& el : scores) {
        write_number<playerid_t>(el.first);
        write_number<score_t>(el.second);
    }
}

scores_t Buffer::read_players_scores_map() {
    auto map_size = read_number<list_and_map_size_t>();
    scores_t new_map;
    for (size_t i = 0; i < map_size; i++) {
        auto key = read_number<playerid_t>();
        auto val = read_number<score_t>();
        new_map.insert({key, val});
    }
    return new_map;
}

// Write Events functions
/*
void Buffer::write_bomb_placed_event(BombPlaced *event) {}
void Buffer::write_bomb_exploded_event(BombExploded *event) {}
void Buffer::write_player_moved_event(PlayerMoved *event) {}
void Buffer::write_block_placed_event(BlockPlaced *event) {}

void Buffer::write_event(Event *event) {
    switch (event->nr) {
        case Events_nr::BombPlaced_nr:
            write_bomb_placed_event((BombPlaced*) event);
            break;
        case Events_nr::BombExploded_nr:
            write_bomb_exploded_event((BombExploded*) event);
            break;
        case Events_nr::PlayerMoved_nr:
            write_player_moved_event((PlayerMoved*) event);
            break;
        case Events_nr::BlockPlaced_nr:
            write_block_placed_event((BlockPlaced*) event);
            break;
        default:
            throw std::invalid_argument("Passed event has invalid nr field");
    }
}

void Buffer::write_events(vector<event_ptr> &events) {
    //todo zapisać rozmiar struktury
    for (auto & event : events)
        write_event(event.get());
}
*/

event_ptr Buffer::read_bomb_placed_event() {
    auto bomb_id = read_number<bombid_t>();
    position_t position = read_position();
    return make_unique<BombPlaced>(bomb_id, position.x, position.y);
}

event_ptr Buffer::read_bomb_exploded_event() {
    auto bomb_id = read_number<bombid_t>();
    players_id_list destroyed_robots = read_players_id_list();
    positions_set blocks_destroyed = read_position_set();
    return make_unique<BombExploded>(bomb_id, destroyed_robots, blocks_destroyed);
}

//todo: poprawić na template z read_bomb_placed_event: template<T1, T2> uptr<Ev> read_.._ev()
event_ptr Buffer::read_player_moved_event() {
    auto player_id = read_number<playerid_t>();
    position_t position = read_position();
    return make_unique<PlayerMoved>(player_id, position.x, position.y);
}

event_ptr Buffer::read_block_placed_event() {
    position_t position = read_position();
    return make_unique<BlockPlaced>(position.x, position.y);
}

event_ptr Buffer::read_event() {
    auto event_number = read_number<uint8_t>();
    switch (event_number) {
        case Events_nr::BombPlaced_nr:
            return read_bomb_placed_event();
        case Events_nr::BombExploded_nr:
            return read_bomb_exploded_event();
        case Events_nr::PlayerMoved_nr:
            return read_player_moved_event();
        case Events_nr::BlockPlaced_nr:
            return read_block_placed_event();
        default:
            throw std::invalid_argument("Event stored in buffer is invalid");
    }
}

events_ptr_list Buffer::read_events() {
    auto events_count = read_number<list_and_map_size_t>();

    events_ptr_list event_list;
    for (size_t i = 0; i < events_count; i++) {
        event_list.push_back(read_event());
    }
    return event_list;
}


