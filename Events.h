//
// Created by tomek on 26.05.2022.
//

#ifndef BOMBOWEROBOTY_KLIENT_EVENTS_H
#define BOMBOWEROBOTY_KLIENT_EVENTS_H
#include "types.h"
#include <iostream>
#include <utility>

enum Events_nr {BombPlaced_nr, BombExploded_nr, PlayerMoved_nr, BlockPlaced_nr};

class Event {
public:
    const uint8_t nr;
    position_t position{};
protected:
    Event(uint8_t event_nr, coordinates_t x, coordinates_t y) : nr(event_nr) {
        position.x = x;
        position.y = y;
    }

};

class BombPlaced : public Event {
public:
    bombid_t bomb_id;

    BombPlaced(bombid_t  bid, coordinates_t x, coordinates_t y)
        : Event(Events_nr::BombPlaced_nr, x, y )
        , bomb_id(bid) {}

//    bomb_t create_bomb_struct_from_event(bomb_timer_t timer) {
//        bomb_t new_bomb{};
//        new_bomb.position = position;
//        new_bomb.timer = timer;
//        new_bomb.id = bomb_id;
//        return new_bomb;
//    }
};

class BombExploded : public Event {
public:
    bombid_t bomb_id;
    players_id_list robots_destroyed;
    positions_set blocks_destroyed;

    BombExploded(bombid_t  bid, players_id_list robots_destroyed,
                 positions_set blocks_destroyed)
    : Event(Events_nr::BombExploded_nr, 0, 0)
    , bomb_id(bid)
    , robots_destroyed(std::move(robots_destroyed))
    , blocks_destroyed(std::move(blocks_destroyed)) {}
};

class PlayerMoved : public Event {
public:
    playerid_t player_id;
    PlayerMoved(playerid_t  pid, coordinates_t x, coordinates_t y)
    : Event(Events_nr::PlayerMoved_nr, x, y)
    , player_id(pid) {}
};

class BlockPlaced : public Event {
public:
    BlockPlaced(coordinates_t x, coordinates_t y)
    : Event(Events_nr::BlockPlaced_nr, x, y) {}
};

using event_ptr = std::unique_ptr<Event>;
using events_ptr_list = vector<event_ptr>;


#endif //BOMBOWEROBOTY_KLIENT_EVENTS_H
