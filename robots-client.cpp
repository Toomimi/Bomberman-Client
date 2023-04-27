#include <iostream>
#include "ProgramParameters.h"
#include "Connection.h"

using namespace std;


int main(int argc, char* argv[]) {
    //todo zamiast exit(1) rzucać wyjątek łapany w MAIN i return 1 (exit nie wywołuje destruktorów)
    parameters_t program_params{argc, argv};

//    cout << (program_params->gui_listen_port) << "\n";
//    cout << (program_params->gui_address) << "\n";
//    cout << (program_params->gui_send_port) << "\n";
//    cout << (program_params->server_address) << "\n";
//    cout << (program_params->server_send_port) << "\n";
//    cout << (program_params->player_name) << "\n";

    shared_ptr<GameParameters> game_params = make_shared<GameParameters>();
    game_params->player_name = program_params.player_name;
    boost::asio::io_context io_context;

    Server_TCP server(game_params, &program_params, io_context);
    GUI_UDP gui(game_params, &program_params, io_context);
    gui.add_server_pointer(&server);
    server.add_gui_ptr(&gui);

    io_context.run();


    return 0;
}