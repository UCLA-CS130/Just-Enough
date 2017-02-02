#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "webserver.h"
#include <sstream>

using ::testing::HasSubstr;
using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using boost::asio::ip::tcp;

class MockWebserver : public Webserver {
    public:
        MOCK_METHOD1(processRawRequest, std::string(std::string));
        MOCK_METHOD2(acceptConnection, bool(tcp::acceptor& acceptor, tcp::socket& sock));
        MOCK_METHOD1(processConnection, void(tcp::socket& sock));

        MockWebserver(unsigned short port)
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
    MockWebserver webserver(8080);

    EXPECT_CALL(webserver, acceptConnection(_, _))
        .Times(3)
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(webserver, processConnection(_))
        .Times(2); // for two accepted connections

    webserver.run();
}

