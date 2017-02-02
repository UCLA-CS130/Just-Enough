#include <boost/asio.hpp>

class Webserver {
    public:
        Webserver(unsigned short port);
        virtual void run();

        virtual bool acceptConnection(boost::asio::ip::tcp::acceptor& acceptor, boost::asio::ip::tcp::socket& socket);
        virtual void processConnection(boost::asio::ip::tcp::socket& socket);
        virtual std::string processRawRequest(std::string& reqStr);

    private:
        boost::asio::io_service io_service_;
        unsigned short port_;
};

