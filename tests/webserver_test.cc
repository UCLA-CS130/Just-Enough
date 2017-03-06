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
        MOCK_METHOD1(acceptConnection, bool(tcp::socket& sock));
        MOCK_METHOD2(processConnection, void(int threadIndex, tcp::socket& sock));
        MOCK_METHOD1(runThread, void(int));

        MockWebserverRun(Options* opt)
            : Webserver(opt)
        { }
};

class MockWebserverRunThread : public Webserver {
    public:
        MOCK_METHOD1(acceptConnection, bool(tcp::socket& sock));
        MOCK_METHOD2(processConnection, void(int threadIndex, tcp::socket& sock));

        MockWebserverRunThread(Options* opt)
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
        MOCK_METHOD2(logConnectionDetails, void(int, tcp::socket& socket));
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

TEST(WebserverTest, startThreads) {
    Options opts;
    opts.port = 8080;
    opts.thread = 8;
    MockWebserverRun webserver(&opts);

    for (size_t i = 0; i < opts.thread; i++) {
        EXPECT_CALL(webserver, runThread(i))
            .Times(1)
            .WillOnce(Return());
    }

    webserver.run();
}


TEST(WebserverTest, acceptConnections) {
    Options opts;
    opts.port = 8080;
    MockWebserverRunThread webserver(&opts);

    EXPECT_CALL(webserver, acceptConnection(_))
        .Times(3)
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(webserver, processConnection(_, _))
        .Times(2); // for two accepted connections

    webserver.runThread(0);
}

TEST(WebserverTest, processConnectionTest) {
    Options opts;
    opts.port = 8080;
    int threadIndex = 0;
    MockWebserverProcessConnection webserver(&opts);

    EXPECT_CALL(webserver, logConnectionDetails(threadIndex, _)).Times(1);

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
    webserver.processConnection(threadIndex, socket);
}

TEST(WebserverTest, matcheRequestWithHandler) {
    Options opts;
    opts.port = 8080;
    RequestHandler* handler = new EchoHandler();
    std::string uri = "/";
    NginxConfig config;
    handler->Init(uri, config);
    opts.handlerMap[uri] = handler;
    Webserver ws(&opts);

    std::string reqStr = (
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "\r\n"
            );
    auto req = Request::Parse(reqStr);

    RequestHandler* foundhandler = ws.matchRequestWithHandler(*req);
    EXPECT_EQ(foundhandler, handler);
}

class WebserverMatcherTest : public ::testing::Test {
    protected:
        // given an array of path prefix to RequestHandler*, initialize a webserver instance
        virtual bool createServerFromHandlers(std::vector<std::pair<std::string, RequestHandler*>> handlers) {
            opts_ = new Options();
            opts_->port = 8080;
            NginxConfig config;
            for (auto& h : handlers) {
                h.second->Init(h.first, config);
                opts_->handlerMap[h.first] = h.second;
            }
            opts_->defaultHandler = nullptr;
            ws_ = new Webserver(opts_);
            return true;
        }

        virtual void Teardown() {
            delete ws_;
            delete opts_;
        }

        Options* opts_;
        Webserver* ws_;
};

TEST_F(WebserverMatcherTest, basic_matchRequestWithHandler) {
    RequestHandler* h1 = new EchoHandler();
    createServerFromHandlers({
            {"/foo", h1},
            });

    auto req = Request::Parse(
            "GET /foo/bar HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "\r\n"
            );
    RequestHandler* foundhandler = ws_->matchRequestWithHandler(*req);
    EXPECT_EQ(foundhandler, h1);

    delete h1;
}

TEST_F(WebserverMatcherTest, multiple_matchRequestWithHandler) {
    RequestHandler* h1 = new EchoHandler();
    RequestHandler* h2 = new EchoHandler();
    createServerFromHandlers({
            {"/foo", h1},
            {"/baz", h2},
            });

    auto req = Request::Parse(
            "GET /foo/bar HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "\r\n"
            );
    RequestHandler* foundhandler = ws_->matchRequestWithHandler(*req);
    EXPECT_EQ(foundhandler, h1);

    delete h1;
    delete h2;
}

TEST_F(WebserverMatcherTest, notfound_matchRequestWithHandler) {
    RequestHandler* h1 = new EchoHandler();
    RequestHandler* h2 = new EchoHandler();
    createServerFromHandlers({
            {"/foo", h1},
            {"/baz", h2},
            });

    auto req = Request::Parse(
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "\r\n"
            );
    RequestHandler* foundhandler = ws_->matchRequestWithHandler(*req);
    EXPECT_EQ(foundhandler, nullptr);

    delete h1;
    delete h2;
}

TEST_F(WebserverMatcherTest, conflict_matchRequestWithHandler) {
    RequestHandler* h1 = new EchoHandler();
    RequestHandler* h2 = new EchoHandler();
    RequestHandler* h3 = new EchoHandler();
    createServerFromHandlers({
            {"/foo", h1},
            {"/foo/bar/baz", h3},
            {"/foo/bar", h2},
            });

    auto req = Request::Parse(
            "GET /foo/bar/baz/bop HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "\r\n"
            );
    RequestHandler* foundhandler = ws_->matchRequestWithHandler(*req);
    EXPECT_EQ(foundhandler, h3);

    delete h1;
    delete h2;
    delete h3;
}

