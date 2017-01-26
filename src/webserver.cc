#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include "options.h"

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
    std::cout << "listening on port 8080" << std::endl;

    try {
        if (argc != 2) {
            std::cerr << "Invalid number of arguments. Usage: webserver <config_file>\n";
            return 1;
        }

        Options opt(argv[1]);
        short port_num = opt.getPort();
        //this can be used to check Options::getPort
        /*if(port_num == 8080) {
            std::cout << "Currently listening" << std::endl;
            return 0;
        }*/

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
