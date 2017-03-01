#pragma once
#include <thread>
#include <boost/asio.hpp>
#include "options.h"
#include "request_handler.h"
#include "multimap_counter.h"

const size_t DEFAULT_NUM_THREADS = 8;

class Webserver {
    public:
        // faux singleton: Webserver::instance stores the most recently constructed webserver.
        // Multiple webserver instances are allowed, but handlers that rely on global status will see the most recent.
        static Webserver* instance;

        Webserver(Options* opt);
        virtual void run();
        virtual void runThread(int threadIndex);

        virtual std::string readStrUntil(
                boost::asio::ip::tcp::socket& socket,
                boost::asio::streambuf& buf,
                const char* termChar,
                boost::system::error_code& err);
        virtual void logConnectionDetails(int threadIndex, boost::asio::ip::tcp::socket& socket);
        virtual bool acceptConnection(boost::asio::ip::tcp::socket& socket);
        virtual void processConnection(int threadIndex, boost::asio::ip::tcp::socket& socket);
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
        Options* opt_;
        boost::asio::io_service io_service_;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;

        std::mutex mtx_; // guards output, TODO(evan): use thread-safe logging instead

        MultiMapCounter<std::string, Response::ResponseCode> counters_;
        std::vector<std::thread> threads_;
};

