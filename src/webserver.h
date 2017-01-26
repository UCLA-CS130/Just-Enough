#include <boost/asio.hpp>

class Webserver {
    public:
        void run();

    private:
        boost::asio::io_service io_service_;
};
