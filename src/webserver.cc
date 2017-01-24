#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

using boost::asio::ip::tcp;

int main() {
    std::cout << "listening on port 1234" << std::endl;

    try {
        boost::asio::io_service io_service;
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 1234));

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
