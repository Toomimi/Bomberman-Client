//
// Created by tomek on 29.05.2022.
//

#ifndef BOMBOWEROBOTY_KLIENT_PROGRAMPARAMETERS_H
#define BOMBOWEROBOTY_KLIENT_PROGRAMPARAMETERS_H

#include <iostream>
#include "boost/program_options.hpp"
#include "types.h"

using std::cout;
namespace po = boost::program_options;


class parameters_t {
private:

    static size_t find_colon(const string& address) {
        size_t delim_id = address.find_last_of(':');
        if (delim_id == SIZE_MAX) {
            cout << "Invalid address: " << address << "\n";
            exit(1);
        }
        return delim_id;
    }

    static string get_ip(const string& address) {
        //TODO dodać sprawdzanie adresów + obsługę IPv6
        size_t delim = find_colon(address);
        return address.substr(0, delim);
    }
    static string get_port(const string& address) {
        size_t delim = find_colon(address);
        return address.substr(delim + 1);
    }
public:
    string gui_address, server_address, player_name;
    string gui_send_port, server_send_port;
    port_t gui_listen_port; //, gui_send_port, server_send_port;

    parameters_t(int argc, char* argv[]) {
        po::options_description desc("Available flags:");
        desc.add_options()
                ("help,h", "get information about program")
                ("player-name,n", po::value<string>(), "<string>")
                ("gui-address,d", po::value<string>(), "<(host_name):(port) or (IPv4):(port) or (IPv6):(port)>")
                ("server-address,s", po::value<string>(), "<(host_name):(port) or (IPv4):(port) or (IPv6):(port)>")
                ("port,p", po::value<uint16_t>(), "<u16> port number, on which clients gets messages from GUI")
                ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            exit(0);
        }

        if (!vm.count("gui-address") || !vm.count("player-name") ||
            !vm.count("port") || !vm.count("server-address") ) {
            cout << "Invalid arguments, all flags are required. Usage: ./client_name [flags]\n"<< desc;
            exit(1); //todo exception
        }
        player_name = vm["player-name"].as<string>();
        gui_address = get_ip(vm["gui-address"].as<string>());
        gui_send_port = get_port(vm["gui-address"].as<string>());
        server_address = get_ip(vm["server-address"].as<string>());
        server_send_port = get_port(vm["server-address"].as<string>());
        gui_listen_port = vm["port"].as<uint16_t>();
    }
};



#endif //BOMBOWEROBOTY_KLIENT_PROGRAMPARAMETERS_H
