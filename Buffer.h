//
// Created by tomek on 21.05.2022.
//

#ifndef BOMBOWEROBOTY_KLIENT_BUFFER_H
#define BOMBOWEROBOTY_KLIENT_BUFFER_H

#include <cstddef>
#include <cstring>
#include <string>
#include <memory>
#include <netinet/in.h>
//#include "types.h"
#include "Events.h"

#define BUFF_SIZE 65535 * 10
#define MAX_TCP 65535
#define MAX_UDP 65507

using std::string;
using std::vector;
using std::make_unique;

class Buffer {
public:
    Buffer();
    ~Buffer();

    void clear();
    void clear_read_message();
    void move_read_index_to_begin();

    void check_if_enough_data(size_t data_size) const;

    char* get_buff_end();
    void update_len(size_t written_size);
    char* get_buff();
    size_t get_len() const;

    void print_buffer(string info) {
        std::cout << info << ": \n";
        char tmp = buff[len];
        buff[len] = '\0';
        std::cout << "len: " << len << "\n";
        std::cout << "buff: ";

        for (size_t i = 0; i < len; i++)
            printf("%d ", (uint8_t) buff[i]);
        std::cout << "\n\n";
        buff[len] = tmp;
    }


    template<typename T>
    void write_number(T el) {
        T big_endian_el = sizeof(T) == 8 ? htobe64(el) :
                          sizeof(T) == 4 ? htonl(el) :
                          sizeof(T) == 2 ? htons(el) : el;
        memcpy(buff + len, &big_endian_el, sizeof(T));
        len += sizeof(T);
    }

    template<typename T>
    T read_number(){
//        check_if_enough_data(sizeof(T));
        T el;
        memcpy(&el, buff + read_index, sizeof(T));
        read_index += sizeof(T);
        el = sizeof(T) == 8 ? be64toh(el) : sizeof(T) == 4 ? ntohl(el) :
                                            sizeof(T) == 2 ? ntohs(el) : el;
        return el;
    }

    void write_string(const string& string_to_add);
    string read_string();

    void write_position_list(positions_list& list);
    positions_list read_position_list();

    void write_position_set(positions_set& set);
    positions_set read_position_set();

    void write_bombs_map(bombs_map_t &map);
    vector<bomb_t> read_bombs_list();

    void write_players_id_list(players_id_list &list);
    players_id_list read_players_id_list();

    void write_players_map(players_t &players);
    players_t read_players_map();

    void write_players_positions_map(players_positions_t &players_positions);
    players_positions_t read_players_positions_map();

    void write_players_scores_map(scores_t &scores);
    scores_t read_players_scores_map();

//    void write_events(vector<unique_ptr<Event>> &events);
    events_ptr_list read_events();

    void write_player(const player_t &player);
    player_t read_player();

private:
    char *buff;
    size_t len, read_index;

    void check_if_structure_fits_and_write_its_len(size_t s_len, size_t el_size);

    void write_position(position_t p);
    position_t read_position();

    void write_bomb(bomb_t bomb);
    bomb_t read_bomb();

//    void write_bomb_placed_event(BombPlaced *event);
//    void write_bomb_exploded_event(BombExploded *event);
//    void write_player_moved_event(PlayerMoved *event);
//    void write_block_placed_event(BlockPlaced *event);
//    void write_event(Event *event);

    event_ptr read_bomb_placed_event();
    event_ptr read_bomb_exploded_event();
    event_ptr read_player_moved_event();
    event_ptr read_block_placed_event();
    event_ptr read_event();
    };

#endif //BOMBOWEROBOTY_KLIENT_BUFFER_H
