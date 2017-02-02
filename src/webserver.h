#include <boost/asio.hpp>

class Webserver {
    public:
        Webserver(unsigned short port);
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

    private:
        boost::asio::io_service io_service_;
        unsigned short port_;
};

