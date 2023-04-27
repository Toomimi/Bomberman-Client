//
// Created by tomek on 20.05.2022.
//

#include "Connection.h"

#include <utility>

namespace b = boost;
namespace ba = b::asio;
using ba::ip::tcp;
using ba::ip::udp;

using std::cerr;
using std::move;

//typedef void (*void_func_t) ();
//void_func_t parse_message_functions_array[] = {}; //todo

Connection::Connection(shared_ptr<GameParameters> game_state)
        : game_state(move(game_state)) {}

/*----------------------------------------------------------------------------*/
// Server_TCP
// Private:
void Server_TCP::parse_bomb_placed_event(BombPlaced *event) {
    bomb_t new_bomb{};
    new_bomb.position = event->position;
    new_bomb.timer = game_state->bomb_timer;
    game_state->bombs.insert({event->bomb_id, new_bomb});
}

bool Server_TCP::check_if_position_on_board(signed_coordinates_t x, signed_coordinates_t y) {
    return x >= 0 && x < game_state->size_x && y >= 0 && y < game_state->size_y;
}

void Server_TCP::add_explosion_position_if_valid(signed_coordinates_t x, signed_coordinates_t y,
                                                 bool* blocked_direction, Direction d,
                                                 positions_set& destroyed_blocks) {
    if (blocked_direction[d] || !check_if_position_on_board(x, y))
        return;

    position_t explosion_p{};
    explosion_p.x = (coordinates_t) x;
    explosion_p.y = (coordinates_t) y;
    positions_set& set = game_state->explosions;
    set.insert(explosion_p);
    if (destroyed_blocks.find(explosion_p) != set.end())
        blocked_direction[d] = true;
}

void Server_TCP::decrease_bombs_timer() {
    for (auto& b : game_state->bombs)
        b.second.timer--;
}

void Server_TCP::parse_bomb_exploded_event(BombExploded *event) {
    for (position_t destroyed_block : event->blocks_destroyed)
        game_state->blocks.erase(destroyed_block);

    for (playerid_t& destroyed_robot : event->robots_destroyed) {
        auto& already_destroyed = game_state->current_turn_destroyed_robots;
        if (already_destroyed.find(destroyed_robot) == already_destroyed.end()) {
            game_state->scores[destroyed_robot]++;
            already_destroyed.insert(destroyed_robot);
        }
    }
    bombid_t bomb_id = event->bomb_id;
    bomb_t& exploded_bomb = game_state->bombs[bomb_id];
    game_state->bombs.erase(bomb_id);
    signed_coordinates_t x = exploded_bomb.position.x, y = exploded_bomb.position.y;

    game_state->explosions.insert(exploded_bomb.position);
    positions_set s = event->blocks_destroyed;
    if (s.find(exploded_bomb.position) != s.end()) // if bomb is on block, don t count explosion radius
        return;
    bool blocked_direction[4] = {0};
    for (signed_coordinates_t i = 1; i <= game_state->explosion_r; i++) {
        add_explosion_position_if_valid(x, y + i, blocked_direction, Up, s);
        add_explosion_position_if_valid(x + i, y, blocked_direction, Right, s);
        add_explosion_position_if_valid(x, y - i, blocked_direction, Down, s);
        add_explosion_position_if_valid(x - i, y, blocked_direction, Left, s);
    }

}

void Server_TCP::parse_player_moved_event(PlayerMoved *event) {
    game_state->players_positions[event->player_id] = event->position;
}

void Server_TCP::parse_block_placed_event(BlockPlaced *event) {
    game_state->blocks.insert(event->position);
}

void Server_TCP::parse_hello_message() {
    string server_name = received_buff.read_string();
    auto players_count = received_buff.read_number<players_count_t>();
    auto size_x = received_buff.read_number<coordinates_t>();
    auto size_y = received_buff.read_number<coordinates_t>();
    auto game_len = received_buff.read_number<game_len_t>();
    auto explosion_r = received_buff.read_number<explosion_radius_t>();
    auto bomb_timer = received_buff.read_number<bomb_timer_t>();
    game_state->save_parameters_from_hello_message(server_name, players_count,
                                                   size_x, size_y, game_len,
                                                   explosion_r, bomb_timer);
}

void Server_TCP::parse_accepted_player_message() {
    auto player_id = received_buff.read_number<playerid_t>();
    player_t new_player = received_buff.read_player();
    game_state->players.insert({player_id, new_player});
}

void Server_TCP::parse_game_started_message() {
    game_state->set_players_map(received_buff.read_players_map());
    game_state->state = GameState::WaitingForTurn0;
}

void Server_TCP::parse_turn_message() {
    decrease_bombs_timer();
    game_state->explosions.clear();

    auto turn_number = received_buff.read_number<turn_number_t>();
    game_state->turnNumber = turn_number;
    if (turn_number == 0)
        game_state->state = GameState::Played;
    events_ptr_list events = received_buff.read_events();
    for (event_ptr & event : events) {
        Event* ev = event.get();
        switch (event->nr) {
            case Events_nr::BombPlaced_nr:
                parse_bomb_placed_event((BombPlaced*) ev);
                break;
            case Events_nr::BombExploded_nr:
                parse_bomb_exploded_event((BombExploded*) ev);
                break;
            case Events_nr::PlayerMoved_nr:
                parse_player_moved_event((PlayerMoved*) ev);
                break;
            case Events_nr::BlockPlaced_nr:
                parse_block_placed_event((BlockPlaced*) ev);
                break;
            default:
                throw std::invalid_argument("Event stored in buffer is invalid");
        }
    }
    game_state->current_turn_destroyed_robots.clear();
}

void Server_TCP::parse_game_ended_message() {
    game_state->scores = received_buff.read_players_scores_map();
    game_state->state = GameState::Lobby_state;
}

void Server_TCP::parse_received_message() {
    auto c = received_buff.read_number<message_nr_t>();
    switch (c) {
        case ServerMessage::Hello:
            received_buff.print_buffer("Received Hello message from server");
            parse_hello_message();
            break;
        case ServerMessage::AcceptedPlayer:
            received_buff.print_buffer("Received AcceptedPlayer message from server");
            parse_accepted_player_message();
            break;
        case ServerMessage::GameStarted:
            received_buff.print_buffer("Received GameStarted message from server");
            parse_game_started_message();
            break;
        case ServerMessage::Turn:
            received_buff.print_buffer("Received Turn message from server");
            parse_turn_message();
            break;
        case ServerMessage::GameEnded:
            received_buff.print_buffer("Received GameEnded message from server");
            parse_game_ended_message();
            game_state->clear_agregated_parameters();
            break;
        default:
            throw std::logic_error("Received message type is invalid.");
    }
    received_buff.clear_read_message();
}

void Server_TCP::send_and_clear_buffer() {
    ba::async_write(socket, ba::buffer(sending_buff.get_buff(), sending_buff.get_len()),
                    [this](b::system::error_code error, std::size_t) {
        if (error) {
            std::cerr << "Sending message to server failed" << '\n';
            exit(1);
        }
    });
    cout << "Sent message to server\n";
    sending_buff.clear();
}

void Server_TCP::receive_message() {
    cout << "Waiting for new messages from server.\n";
    socket.async_receive(ba::buffer(received_buff.get_buff_end(), MAX_TCP),
                         [this](b::system::error_code error, std::size_t written_size) {
            if (error) {
                std::cerr << error.message() << '\n';
                exit(1);
            }
            received_buff.update_len(written_size);
            try {
                parse_received_message();
            }
            catch (std::exception &e) {
                received_buff.print_buffer("Received invalid or incomplete msg");
//                try {
                    throw e;
//                }
//                catch(const std::underflow_error &e) {
//                    received_buff.move_read_index_to_begin();
//                    receive_message();
//                    return;
//                }
            }

            if (game_state->state == GameState::Lobby_state)
                gui->send_lobby_message();
            else if (game_state->state == GameState::Played)
                gui->send_game_message();

            receive_message();
            return;
    });
}

void Server_TCP::send_move_message(Direction d) {
    sending_buff.write_number<message_nr_t>(Move);
    sending_buff.write_number<uint8_t>(d);
    sending_buff.print_buffer("Sending move message to server");
    send_and_clear_buffer();
}

void Server_TCP::send_place_bomb_message() {
    sending_buff.write_number<message_nr_t>(PlaceBomb);
    sending_buff.print_buffer("Sending PlaceBomb message to server");
    send_and_clear_buffer();
}

void Server_TCP::send_place_block_message() {
    sending_buff.write_number<message_nr_t>(PlaceBlock);
    sending_buff.print_buffer("Sending PlaceBlock message to server");
    send_and_clear_buffer();
}

// Public:
Server_TCP::Server_TCP(shared_ptr<GameParameters> game_state, parameters_t* prg_params, ba::io_context& io_context)
    : Connection(move(game_state)), socket(io_context)  {
    try {
        string address = prg_params->server_address, port = prg_params->server_send_port;
        cout << "Connecting to server " << address << ":" << port << "\n";
        tcp::resolver resolver(io_context);
        auto endpoints = *resolver.resolve(address, port);
        socket.connect(endpoints);
        cout << "Connected to server.\n";
        socket.set_option(ba::ip::tcp::no_delay(true));
        receive_message();
    } catch (std::exception &e) {
        cerr << "Could not connect to the server: " << e.what() << '\n';
        exit(1);
    }
}

void Server_TCP::add_gui_ptr(GUI_UDP* gui_ptr) {
    gui = gui_ptr;
}

void Server_TCP::send_join_message(string& player_name) {
    sending_buff.write_number<message_nr_t>(Join);
    sending_buff.write_string(player_name);
    sending_buff.print_buffer("Sending Join message to server");
    send_and_clear_buffer();
}

void Server_TCP::send_player_action_from_gui_message(InputMessage& msg) {
    switch(msg.type) {
        case PlaceBomb_in:
            send_place_bomb_message();
            break;
        case PlaceBlock_in:
            send_place_block_message();
            break;
        case Move_in:
            send_move_message(msg.direction);
            break;
        default:
            throw std::invalid_argument("Invalid message type.");
    }
}


/*---------------------------------------------------------------------------------------------------------------------------*/
// GUI_UDP
// Private:
void GUI_UDP::send_and_clear_buffer() {
    socket.async_send_to(ba::buffer(sending_buff.get_buff(), sending_buff.get_len()),
                         endpoint,
                         [this](boost::system::error_code error, std::size_t) {
                if (error) {
                    cerr << "Sending message to GUI failed.\n";
                    exit(1);
                }
            });
    cout << "Sent message to GUI\n";
    sending_buff.clear();
}

void GUI_UDP::parse_received_message(InputMessage& msg, size_t msg_len) {
    auto msg_type = received_buff.read_number<message_nr_t>();
    if (msg_type >= PlaceBomb_in && msg_type <= Move_in) {
        msg.type = (InputMessage_nr) msg_type;
        if (msg.type == Move_in) {
            if (msg_len != 2)
                msg.type = NotAvailableOrInvalid_in;
            else
                msg.direction = (Direction) received_buff.read_number<uint8_t>();
        }
        else if (msg_len != 1) {  //msg.type is different input message
            msg.type = NotAvailableOrInvalid_in;
        }
    }
    received_buff.clear();
}

// Public:
GUI_UDP::GUI_UDP(shared_ptr<GameParameters> game_state, parameters_t* prg_params, ba::io_context& io_context)
        : Connection(move(game_state)), socket(io_context, udp::endpoint(udp::v6(), prg_params->gui_listen_port)) {
    try {
        cout << "created socket for Gui listen on port: " << prg_params->gui_listen_port << '\n';
        string address = prg_params->gui_address, port = prg_params->gui_send_port;
        udp::resolver resolver(io_context);
        cout << "Creating endpoint for GUI " << address << ":" << port << "\n";
        endpoint = *resolver.resolve(prg_params->gui_address, prg_params->gui_send_port);
        cout << "Created GUI endpoint.\n";
        receive_message();
    } catch (std::exception &e) {
        cerr << "Could not connect to GUI: " << e.what() << '\n';
        exit(1);
    }
}

void GUI_UDP::add_server_pointer(Server_TCP* server_ptr) {
    server = server_ptr;
}

void GUI_UDP::receive_message() {
    cout << "Waiting for messages from GUI.\n";
    socket.async_receive_from(ba::buffer(received_buff.get_buff(), MAX_UDP), endpoint,
                              [this](b::system::error_code error, size_t msg_len) {
        if (error) {
            cerr << error.message() << '\n';
            exit(1);
        }
        received_buff.update_len(msg_len);
        received_buff.print_buffer("Received message from GUI");
        InputMessage new_msg(NotAvailableOrInvalid_in);
        parse_received_message(new_msg, msg_len);
        if (new_msg.type != NotAvailableOrInvalid_in) {
            if (game_state->state == GameState::Lobby_state )
                server->send_join_message(game_state->player_name);
            else if (game_state->state == GameState::Played)
                server->send_player_action_from_gui_message(new_msg);
        }
        receive_message();
    });
}

void GUI_UDP::send_lobby_message() {
    game_state->save_to_buffer_gui_lobby_message(sending_buff);
    sending_buff.print_buffer("Sending lobby message to GUI");
    send_and_clear_buffer();
}

void GUI_UDP::send_game_message() {
    game_state->save_to_buffer_gui_game_message(sending_buff);
    sending_buff.print_buffer("Sending game message to GUI");
    send_and_clear_buffer();
}
