#include <string>
#include <sstream>
#include <iostream>
#include <boost/system/error_code.hpp>

//#include "http_request.h"
//#include "http_response.h"
#include "request_handler.h"
#include "webserver.h"

using boost::asio::ip::tcp;

std::string Webserver::processRawRequest(std::string& reqStr) {
    auto req = Request::Parse(reqStr);
    Response resp;

    bool handled = false;

    // TODO(evan): stop using brute force prefix matching
    std::string prefix = req->uri();
    for (int prefixSize = prefix.size(); prefixSize > 0; prefixSize--) {
        prefix.resize(prefixSize);

        std::cout << "checking prefix '" << prefix << "'" << std::endl;
        auto match = opt_->handlerMap.find(prefix);
        if (match != opt_->handlerMap.end()) {
            RequestHandler* handler = match->second;
            std::cout << " > matched '" << handler << "'" << std::endl;

            handled = true;
            handler->HandleRequest(*req, &resp);
            break;
        }
    }

    if ( ! handled) {
        std::cerr << "No handler to handle request to " << req->uri() << std::endl;
        resp.SetStatus(Response::code_404_not_found);
        resp.SetBody("404 Not Found");
    }

    return resp.ToString();
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
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), opt_->port));

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

Webserver::Webserver(Options* opt)
: opt_(opt)
{ }
