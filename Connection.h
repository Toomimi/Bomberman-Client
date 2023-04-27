#ifndef BOMBOWEROBOTY_KLIENT_CONNECTION_H
#define BOMBOWEROBOTY_KLIENT_CONNECTION_H

#include <boost/asio.hpp>
#include "Buffer.h"
#include "GameParameters.h"
#include "ProgramParameters.h"

using std::shared_ptr;

class Connection {
private:
    virtual void send_and_clear_buffer() = 0;

protected:
    shared_ptr<GameParameters> game_state;
    Buffer received_buff{}, sending_buff{};
public:
    Connection(shared_ptr<GameParameters> game_state);
    virtual ~Connection() = default;
};

class GUI_UDP;

class Server_TCP : public Connection {
private:
    GUI_UDP* gui;
    boost::asio::ip::tcp::socket socket;

    void add_explosion_position_if_valid(signed_coordinates_t x, signed_coordinates_t y,
                                         bool* blocked_direction, Direction d,
                                         positions_set& destroyed_blocks);
    bool check_if_position_on_board(signed_coordinates_t x, signed_coordinates_t y);
    void decrease_bombs_timer();

    void parse_bomb_placed_event(BombPlaced *event);
    void parse_bomb_exploded_event(BombExploded *event);
    void parse_player_moved_event(PlayerMoved *event);
    void parse_block_placed_event(BlockPlaced *event);

    void parse_hello_message();
    void parse_accepted_player_message();
    void parse_game_started_message();
    void parse_turn_message();
    void parse_game_ended_message();
    void parse_received_message();
    void send_and_clear_buffer() override;

    void send_move_message(Direction d);
    void send_place_bomb_message();
    void send_place_block_message();
public:
    Server_TCP(shared_ptr<GameParameters> game_state, parameters_t* prg_params,
               boost::asio::io_context& io_context);

    void add_gui_ptr(GUI_UDP* gui_ptr);

    void receive_message();

    void send_join_message(string& player_name);
    void send_player_action_from_gui_message(InputMessage &msg);
};

class GUI_UDP : public Connection {
private:
    Server_TCP* server;
    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint endpoint;

    void send_and_clear_buffer() override;
    void parse_received_message(InputMessage& msg, size_t msg_len);
    void receive_message();

public:
    GUI_UDP(shared_ptr<GameParameters> game_state, parameters_t* prg_params,
            boost::asio::io_context& io_context);

    void add_server_pointer(Server_TCP* server_ptr);

    void send_lobby_message();
    void send_game_message();
};


#endif //BOMBOWEROBOTY_KLIENT_CONNECTION_H
