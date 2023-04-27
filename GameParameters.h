//
// Created by tomek on 26.05.2022.
//

#ifndef BOMBOWEROBOTY_KLIENT_GAMEPARAMETERS_H
#define BOMBOWEROBOTY_KLIENT_GAMEPARAMETERS_H
#include <utility>

#include "types.h"
#include "Buffer.h"


enum GameState {Lobby_state, Played, WaitingForTurn0};

class GameParameters {
private:
    void save_lobby_and_game_shared_params_to_buffer(Buffer &b, bool lobby_msg) const {
        b.write_string(server_name);
        if (lobby_msg)
            b.write_number<players_count_t>(players_count);
        b.write_number<coordinates_t>(size_x);
        b.write_number<coordinates_t>(size_y);
        b.write_number<game_len_t>(game_len);
    }

public:
    //otrzymywane w parametrach programu
    string player_name;

    // otrzymywane w hello
    string server_name;
    players_count_t players_count;
    coordinates_t size_x, size_y;
    game_len_t game_len;
    explosion_radius_t explosion_r;
    bomb_timer_t bomb_timer;

    // Potrzebne do działania programu
    GameState state = GameState::Lobby_state;

    // otrzymywane w trakcie przygotowania do rozgrywki w playerAccepted i zaraz przed rozpoczęciem rozgrywki
    players_t players{};

    // otrzymywane na początku rozgrywki i zmieniane na bieżąco
    players_positions_t players_positions{};
    positions_set blocks{};
    turn_number_t  turnNumber;

    //wyliczane na bieżąco w trakcie i agregowane
    scores_t scores{};
    bombs_map_t bombs{};

    // obliczane w trakcie parsowania, a następnie zerowane
    positions_set explosions{};
    players_id_set current_turn_destroyed_robots{};

    void clear_agregated_parameters() {
        bombs.clear();
        scores.clear();
        players.clear();
    }

    void set_players_map(players_t players_map) {
        players = std::move(players_map);
        for (auto& player : players)
            scores.insert({player.first, 0});
    }

    void save_parameters_from_hello_message(string serverName, players_count_t playersCount,
                                            coordinates_t sizeX, coordinates_t sizeY, game_len_t gameLen,
                                            explosion_radius_t explosionR, bomb_timer_t bombTimer) {
        server_name = std::move(serverName);
        players_count = playersCount;
        size_x = sizeX;
        size_y = sizeY;
        game_len = gameLen;
        explosion_r = explosionR;
        bomb_timer = bombTimer;
    }

    void save_to_buffer_gui_lobby_message(Buffer &b) {
        b.write_number<message_nr_t>(GuiMessage::Lobby);
        save_lobby_and_game_shared_params_to_buffer(b, true);
        b.write_number<explosion_radius_t>(explosion_r);
        b.write_number<bomb_timer_t>(bomb_timer);
        b.write_players_map(players);
    }

    void save_to_buffer_gui_game_message(Buffer &b) {
        b.write_number<message_nr_t>(GuiMessage::Game);
        save_lobby_and_game_shared_params_to_buffer(b, false);
        b.write_number<turn_number_t>(turnNumber);
//        b.print_buffer("-----------------------------------Saved_turn number");
        b.write_players_map(players);
//        b.print_buffer("-----------------------------------Saved players_map");
        b.write_players_positions_map(players_positions);
//        b.print_buffer("-----------------------------------Saved players_position_map");
        b.write_position_set(blocks);
//        b.print_buffer("-----------------------------------Saved blocks");
        b.write_bombs_map(bombs);
//        b.print_buffer("-----------------------------------Saved bombs");
        b.write_position_set(explosions);
//        b.print_buffer("-----------------------------------Saved explosions");
        b.write_players_scores_map(scores);
//        b.print_buffer("-----------------------------------Saved scores");

    }


};

#endif //BOMBOWEROBOTY_KLIENT_GAMEPARAMETERS_H
