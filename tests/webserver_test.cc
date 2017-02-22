#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "echo_handler.h"
#include "options.h"
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

        MockWebserverRun(Options* opt)
            : Webserver(opt)
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

        MockWebserverProcessConnection(Options* opt)
            : Webserver(opt)
        { }
};

TEST(WebserverTest, processRawRequest) {
    Options opts;
    opts.port = 8080;
    RequestHandler* handler = new EchoHandler();
    std::string uri = "/";
    NginxConfig config;
    handler->Init(uri, config);
    opts.handlerMap[uri] = handler;
    Webserver ws(&opts);

    std::string req = (
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "\r\n"
            );


    std::string resp = ws.processRawRequest(req);

    EXPECT_THAT(resp, HasSubstr("HTTP/1.1 200 OK\r\n"));
    EXPECT_THAT(resp, HasSubstr("Content-Type: text/plain\r\n"));

    // For now, we're expecting it to echo the request
    EXPECT_THAT(resp, HasSubstr(req));
}


TEST(WebserverTest, acceptConnections) {
    Options opts;
    opts.port = 8080;
    MockWebserverRun webserver(&opts);

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
    Options opts;
    opts.port = 8080;
    MockWebserverProcessConnection webserver(&opts);

    EXPECT_CALL(webserver, logConnectionDetails(_)).Times(1);

    std::string s = "0123456789\r\n";
    EXPECT_CALL(webserver, readStrUntil(_, _, StrEq("\r\n"), _))
        .WillOnce(Return(s))
        .WillOnce(Return(s))
        .WillOnce(Return(s))
        .WillOnce(Return(s))
        .WillOnce(Return(std::string("\r\n")));

    EXPECT_CALL(webserver, processRawRequest(_)).Times(1);

    EXPECT_CALL(webserver, writeResponseString(_, _)).Times(1);

    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket(io_service);
    webserver.processConnection(socket);
}

