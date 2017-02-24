#pragma once
#include <boost/asio.hpp>
#include "options.h"
#include "request_handler.h"
#include "multimap_counter.h"

class Webserver {
    public:
        // faux singleton: Webserver::instance stores the most recently constructed webserver.
        // Multiple webserver instances are allowed, but handlers that rely on global status will see the most recent.
        static Webserver* instance;

        Webserver(Options* opt);
        virtual void run();

        virtual std::string readStrUntil(
                boost::asio::ip::tcp::socket& socket,
                boost::asio::streambuf& buf,
                const char* termChar,
                boost::system::error_code& err);
        virtual void logConnectionDetails(boost::asio::ip::tcp::socket& socket);
        virtual bool acceptConnection(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::ip::tcp::socket& socket);
        virtual void processConnection(boost::asio::ip::tcp::socket& socket);
        virtual std::string processRawRequest(std::string& reqStr);
        virtual void writeResponseString(boost::asio::ip::tcp::socket& socket, const std::string& s);
        virtual RequestHandler* matchRequestWithHandler(const Request& req);

        virtual Options* options() const {
            return opt_;
        }
        virtual MultiMapCounter<std::string, Response::ResponseCode>* counters() {
            return &counters_;
        }

    private:
        boost::asio::io_service io_service_;
        Options* opt_;
        MultiMapCounter<std::string, Response::ResponseCode> counters_;
};

