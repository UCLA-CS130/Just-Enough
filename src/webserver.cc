#include <string>
#include <boost/system/error_code.hpp>

#include "webserver.h"

using boost::asio::ip::tcp;

void Webserver::run() {
    try {
        boost::asio::io_service io_service;
        short port_num = 1234; // TODO: read from opt
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port_num));

        while (true) {
            tcp::socket socket(io_service_);
            acceptor.accept(socket);
            std::cout << "Accepted connection from "
                << socket.remote_endpoint().address().to_string()
                << ":" << socket.remote_endpoint().port()
                << std::endl;

            std::string msg = "Yes, hello\n";

            boost::system::error_code ignored_error;
            boost::asio::write(socket, boost::asio::buffer(msg), ignored_error);
        }
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

