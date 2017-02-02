#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "webserver.h"
#include <sstream>

using ::testing::HasSubstr;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::MatchesRegex;
using boost::asio::ip::tcp;

class MockWebserverRun : public Webserver {
    public:
        MOCK_METHOD2(acceptConnection, bool(tcp::acceptor& acceptor, tcp::socket& sock));
        MOCK_METHOD1(processConnection, void(tcp::socket& sock));

        MockWebserverRun(unsigned short port)
            : Webserver(port)
        { }
};

class MockWebserverProcessConnection : public Webserver {
    public:
        MOCK_METHOD1(processRawRequest, std::string(std::string&));
        MOCK_METHOD4(readStrUntil, std::string(
                boost::asio::ip::tcp::socket& socket,
                boost::asio::streambuf& buf,
                const char* termChar,
                boost::system::error_code& err));
        MOCK_METHOD1(logConnectionDetails, void(tcp::socket& socket));
        MOCK_METHOD2(writeResponseString, void(boost::asio::ip::tcp::socket& socket, const std::string& s));

        MockWebserverProcessConnection(unsigned short port)
            : Webserver(port)
        { }
};

TEST(WebserverTest, processRawRequest) {
    Webserver ws(8080);

    std::string req = (
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "\r\n"
            );


    std::string resp = ws.processRawRequest(req);

    EXPECT_THAT(resp, HasSubstr("HTTP/1.1 200 OK\r\n"));
    EXPECT_THAT(resp, HasSubstr("Content-type: text/plain\r\n"));

    // For now, we're expecting it to echo the request
    EXPECT_THAT(resp, HasSubstr(req));
}


TEST(WebserverTest, acceptConnections) {
    MockWebserverRun webserver(8080);

    EXPECT_CALL(webserver, acceptConnection(_, _))
        .Times(3)
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(webserver, processConnection(_))
        .Times(2); // for two accepted connections

    webserver.run();
}

TEST(WebserverTest, processConnectionTest) {
    MockWebserverProcessConnection webserver(8080);

    EXPECT_CALL(webserver, logConnectionDetails(_)).Times(1);

    std::string s = "0123456789\r\n";
    EXPECT_CALL(webserver, readStrUntil(_, _, StrEq("\r\n"), _))
        .WillOnce(Return(s))
        .WillOnce(Return(s))
        .WillOnce(Return(s))
        .WillOnce(Return(s))
        .WillOnce(Return(std::string("\r\n")))
        ;

    EXPECT_CALL(webserver, processRawRequest(
                _
                //MatchesRegex("^(0123456789\\r\\n){4}\\r\\n$")
                )).Times(1);

    EXPECT_CALL(webserver, writeResponseString(_, _)).Times(1);


    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket(io_service);
    webserver.processConnection(socket);
}

