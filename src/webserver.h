#include <boost/asio.hpp>

class Webserver {
    public:
        Webserver(short port);
        void run();

    private:
        boost::asio::io_service io_service_;
        short port_;
};

std::string processRawRequest(std::string& reqStr);
