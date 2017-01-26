#include <string>
#include <sstream>
#include <boost/system/error_code.hpp>

#include "webserver.h"

using boost::asio::ip::tcp;

std::string processRawRequest(std::string& reqStr) {
    std::string response = reqStr;
    return response;
}

void processConnection(tcp::socket& socket) {
    std::cout << "Accepted connection from "
        << socket.remote_endpoint().address().to_string()
        << ":" << socket.remote_endpoint().port()
        << std::endl;

    boost::asio::streambuf buf;
    std::string req;
    while (true) {
        boost::system::error_code err;
        size_t bytes_read = read_until(socket, buf, "\r\n", err);

        std::istream input(&buf);
        std::string line(bytes_read, ' ');
        input.read(&line[0], bytes_read);
        req += line;

        if (line.size() <= 2) {
            break;
        }

    }
    std::string response = processRawRequest(req);

    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(response), ignored_error);
}

void Webserver::run() {
    try {
        boost::asio::io_service io_service;
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port_));

        while (true) {
            tcp::socket socket(io_service_);
            acceptor.accept(socket);

            processConnection(socket);
        }
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

Webserver::Webserver(short port)
: port_(port)
{ }
