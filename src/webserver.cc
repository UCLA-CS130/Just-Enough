#include <string>
#include <sstream>
#include <iostream>
#include <boost/system/error_code.hpp>

#include "request_handler.h"
#include "webserver.h"

using boost::asio::ip::tcp;

// perform longest prefix matching and return matching handler, or default handler
RequestHandler* Webserver::matchRequestWithHandler(const Request& req) {
    // TODO(evan): stop using brute force prefix matching
    std::string prefix = req.uri();
    for (int prefixSize = prefix.size(); prefixSize > 0; prefixSize--) {
        prefix.resize(prefixSize);

        std::cout << "checking prefix '" << prefix << "'" << std::endl;
        auto match = opt_->handlerMap.find(prefix);
        if (match != opt_->handlerMap.end()) {
            RequestHandler* handler = match->second;
            std::cout << " > matched '" << handler << "'" << std::endl;

            return handler;
        }
    }
    std::cout << "using default handler for " << req.uri() << "\n";
    return opt_->defaultHandler;
}

std::string Webserver::processRawRequest(std::string& reqStr) {
    auto req = Request::Parse(reqStr);
    Response resp;

    RequestHandler* handler = matchRequestWithHandler(*req);

    handler->HandleRequest(*req, &resp);

    counters_.increment(req->uri(), resp.status());

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

inline void Webserver::logConnectionDetails(int threadIndex, tcp::socket& socket) {
    std::unique_lock<std::mutex> lck(mtx_);
    std::cout << "Thread " << threadIndex << " accepted connection from "
        << socket.remote_endpoint().address().to_string()
        << ":" << socket.remote_endpoint().port()
        << std::endl;
}

inline void Webserver::writeResponseString(tcp::socket& socket, const std::string& str) {
    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(str), ignored_error);
}

void Webserver::processConnection(int threadIndex, tcp::socket& socket) {
    logConnectionDetails(threadIndex, socket);

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

bool Webserver::acceptConnection(tcp::socket& socket) {
    try {
        acceptor_->accept(socket);
    } catch (boost::system::system_error& err) {
        std::cerr << err.what() << std::endl;
        return false;
    }
    return true;
}

void Webserver::run() {
    acceptor_ = std::unique_ptr<tcp::acceptor>(new tcp::acceptor(io_service_, tcp::endpoint(tcp::v4(), opt_->port)));

    for (int i = 0; i < DEFAULT_NUM_THREADS; i++) {
        threads_.emplace_back(std::thread(&Webserver::runThread, this, i));
    }
    for (int i = 0; i < (int)threads_.size(); i++) {
        threads_[i].join();
        std::unique_lock<std::mutex> lck(mtx_);
        std::cout << "Thread " << i << " exited"  << std::endl;
    }
}

void Webserver::runThread(int threadIndex) {
    try {
        while (true) {
            tcp::socket socket(io_service_);

            if ( !  acceptConnection(socket)) {
                return;
            }

            processConnection(threadIndex, socket);
        }
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

Webserver* Webserver::instance = nullptr;
Webserver::Webserver(Options* opt)
: opt_(opt), io_service_()
{
    Webserver::instance = this;
}

