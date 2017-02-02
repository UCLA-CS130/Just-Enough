#include <string>
#include <sstream>
#include <boost/system/error_code.hpp>

#include "webserver.h"

using boost::asio::ip::tcp;

std::string Webserver::processRawRequest(std::string& reqStr) {
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-type: text/plain\r\n";
    response += "\r\n"+reqStr+"\r\n";
    return response;
}

inline std::string Webserver::readStrUntil(
        tcp::socket& socket,
        boost::asio::streambuf& buf,
        const char* termChar,
        boost::system::error_code& err)
{
    size_t bytes_read = read_until(socket, buf, termChar, err);
    std::istream input(&buf);
    std::string line(bytes_read, ' ');
    input.read(&line[0], bytes_read);

    return line;
}

inline void Webserver::logConnectionDetails(tcp::socket& socket) {
    std::cout << "Accepted connection from "
        << socket.remote_endpoint().address().to_string()
        << ":" << socket.remote_endpoint().port()
        << std::endl;
}

inline void Webserver::writeResponseString(tcp::socket& socket, const std::string& str) {
    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(str), ignored_error);
}

void Webserver::processConnection(tcp::socket& socket) {
    logConnectionDetails(socket);

    boost::asio::streambuf buf;
    std::string req;
    while (true) {
        boost::system::error_code err;
        std::string line = readStrUntil(socket, buf, "\r\n", err);
        req += line;

        if (line.size() <= 2) {
            break;
        }
    }
    std::string response = processRawRequest(req);

    writeResponseString(socket, response);
}

bool Webserver::acceptConnection(tcp::acceptor& acceptor, tcp::socket& socket) {
    try {
        acceptor.accept(socket);
    } catch (boost::system::system_error& err) {
        std::cerr << err.what() << std::endl;
        return false;
    }
    return true;
}

void Webserver::run() {
    try {
        boost::asio::io_service io_service;
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port_));

        while (true) {
            tcp::socket socket(io_service_);

            if ( !  acceptConnection(acceptor, socket)) {
                return;
            }

            processConnection(socket);
        }
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

Webserver::Webserver(unsigned short port)
: port_(port)
{ }
