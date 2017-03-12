#include <string>
#include <sstream>
#include <iostream>
#include <boost/system/error_code.hpp>

#include "request_handler.h"
#include "base64.h"
#include "utils.h"
#include "webserver.h"

using boost::asio::ip::tcp;

RequestHandler* Webserver::matchRequestWithHandler(const Request& req) {
    // TODO(evan): stop using brute force prefix matching
    std::string prefix = req.uri();
    for (int prefixSize = prefix.size(); prefixSize > 0; prefixSize--) {
        prefix.resize(prefixSize);

        auto match = opt_->handlerMap.find(prefix);
        if (match != opt_->handlerMap.end()) {
            RequestHandler* handler = match->second;

            return handler;
        }
    }
    std::cout << "using default handler for " << req.uri() << "\n";
    return opt_->defaultHandler;
}

std::string Webserver::processRawRequest(std::string& reqStr) {
    auto req = Request::Parse(reqStr);
    if (req == nullptr) {
        return "";
    }
    Response resp;

    std::cout << "request to '" << req->uri() << "'\n";

    auto headers = req->headers();
    bool auth = false;
    for (auto header : headers) {
        if (header.first == "Authorization") {

            std::string b64_part;
            auto tokens = split(header.second, ' ');
            if (tokens.size() != 2 || tokens[0] != "Basic") {
                std::cout << "unrecognized auth format: " << (tokens.size() > 0 ? tokens[0] : "<none>") << std::endl;
                resp.SetStatus(Response::code_401_unauthorized);
                return resp.ToString();
            }

            std::string authUserPass = base64_decode(tokens[1]);
            auto userpass = split(authUserPass, ':');
            if (userpass.size() != 2) {
                std::cout << "unrecognized auth format " << std::endl;
                resp.SetStatus(Response::code_401_unauthorized);
                return resp.ToString();
            }
            std::string user = userpass[0];
            std::string pass = userpass[1];

            if ( ! (user == "admin" && pass == "password")) {
                resp.SetStatus(Response::code_401_unauthorized);
                return resp.ToString();
            }

            std::cout << "authenticated." << std::endl;
            auth = true;
        }
    }
    if ( ! auth) {
        resp.SetStatus(Response::code_401_unauthorized);
        resp.AddHeader("WWW-Authenticate", "Basic realm=\"MyServerRealm\"");
        return resp.ToString();
    }

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
    if (response == "") {
        std::cout << "Error processing request\n";
        return;
    }

    writeResponseString(socket, response);
}

bool Webserver::acceptConnection(tcp::socket& socket) {
    try {
        std::unique_lock<std::mutex> lck(mtx_); // note: don't use this mutex outside of here
        acceptor_->accept(socket);
    } catch (boost::system::system_error& err) {
        std::cerr << err.what() << std::endl;
        return false;
    }
    return true;
}

void Webserver::run() {
    acceptor_ = std::unique_ptr<tcp::acceptor>(new tcp::acceptor(io_service_, tcp::endpoint(tcp::v4(), opt_->port)));

    for (int i = 0; i < (int)opt_->thread; i++) {
        threads_.emplace_back(std::thread(&Webserver::runThread, this, i));
    }
    for (int i = 0; i < (int)threads_.size(); i++) {
        threads_[i].join();
        std::unique_lock<std::mutex> lck(mtx_);
        std::cout << "Thread " << i << " exited"  << std::endl;
    }
}

void Webserver::runThread(int threadIndex) {
    running_ = true;
    try {
        while (running_) {
            tcp::socket socket(io_service_);

            if ( !  acceptConnection(socket)) {
                continue;
            }

            processConnection(threadIndex, socket);
        }
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void Webserver::stop() {
    running_ = false;
}

Webserver* Webserver::instance = nullptr;
Webserver::Webserver(Options* opt)
: opt_(opt), io_service_()
{
    Webserver::instance = this;
}

