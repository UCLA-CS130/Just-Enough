#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include "config_parser.h"

using boost::asio::ip::tcp;

short getPort(NginxConfig* config) {
    //TODO: Find server listen port non-arbitrarily
    std::string p = config->statements_[0]->child_block_->statements_[0]->tokens_[1];
    return (short) std::stoi(p);

    //TODO:: What if port couldn't be found
}

int main(int argc, char* argv[]) {
    //std::cout << "listening on port 1234" << std::endl;

    try {
        if (argc != 2) {
            std::cerr << "Invalid number of arguments. Usage: webserver <config_file>\n";
            return 1;
        }

        NginxConfigParser parser;
        NginxConfig config;

        parser.Parse(argv[1], &config);
        short port_num = getPort(&config);

        boost::asio::io_service io_service;
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port_num));

        while (true) {
            tcp::socket socket(io_service);
            acceptor.accept(socket);

            std::string msg = "Yes, hello\n";

            boost::system::error_code ignored_error;
            boost::asio::write(socket, boost::asio::buffer(msg), ignored_error);
        }
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
