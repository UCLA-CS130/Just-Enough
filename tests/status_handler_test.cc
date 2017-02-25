#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "echo_handler.h"
#include "status_handler.h"
#include "not_found_handler.h"
#include "options.h"
#include "webserver.h"
#include <map>
#include <memory>
#include <string>

using std::map;
using std::string;
using ::testing::HasSubstr;

TEST(RequestHandlerTest, createStatusHandler) {
    std::unique_ptr<RequestHandler> handler(RequestHandler::CreateByName("StatusHandler"));
    ASSERT_NE(handler, nullptr);
}

TEST(StatusHandlerTest, initHandler) {
    std::unique_ptr<RequestHandler> handler(new StatusHandler());
    ASSERT_NE(handler, nullptr);

    NginxConfig config;
    RequestHandler::Status status = handler->Init("/foo", config);
    EXPECT_EQ(status, RequestHandler::OK);
}

class StatusHandlerTestF : public ::testing::Test {
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
            opts_->defaultHandler = new NotFoundHandler();
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

TEST_F(StatusHandlerTestF, handleRequest) {
    RequestHandler* h1 = new EchoHandler();
    RequestHandler* h2 = new EchoHandler();
    RequestHandler* s1 = new StatusHandler();
    createServerFromHandlers({
            {"/foo", h1},
            {"/baz", h2},
            {"/status", s1},
            });


    auto req = Request::Parse(
            "GET /status HTTP/1.1\r\n"
            "\r\n"
            );

    Response resp;
    ASSERT_EQ(s1->HandleRequest(*req, &resp), RequestHandler::OK);

    string respStr = resp.ToString();
    EXPECT_THAT(respStr, HasSubstr("200 OK"));
    EXPECT_THAT(respStr, HasSubstr("/foo"));
    EXPECT_THAT(respStr, HasSubstr("/baz"));
    EXPECT_THAT(respStr, HasSubstr("EchoHandler"));
    EXPECT_THAT(respStr, HasSubstr("/status"));
    EXPECT_THAT(respStr, HasSubstr("StatusHandler"));

    delete h1;
    delete h2;
    delete s1;
}

TEST_F(StatusHandlerTestF, noServer) {
    Webserver::instance = nullptr; // may have been created by previous tests
    RequestHandler* s1 = new StatusHandler();

    auto req = Request::Parse(
            "GET /status HTTP/1.1\r\n"
            "\r\n"
            );

    Response resp;
    ASSERT_EQ(s1->HandleRequest(*req, &resp), RequestHandler::Error);
    delete s1;
}

TEST_F(StatusHandlerTestF, counts) {
    RequestHandler* h1 = new EchoHandler();
    RequestHandler* h2 = new EchoHandler();
    RequestHandler* s1 = new StatusHandler();
    createServerFromHandlers({
            {"/foo", h1},
            {"/bar", h2},
            {"/status", s1},
            });

    // update counters an usual amount of times
    std::string req1 = "GET /foo HTTP/1.1\r\n\r\n";
    std::string req2 = "GET /bar HTTP/1.1\r\n\r\n";
    std::string req3 = "GET /status HTTP/1.1\r\n\r\n";
    for(int i = 0; i < 7; i++) {
        ws_->processRawRequest(req1);
    }
    for(int i = 0; i < 13; i++) {
        ws_->processRawRequest(req2);
    }
    string respStr = ws_->processRawRequest(req3);

    EXPECT_THAT(respStr, HasSubstr("200 OK"));
    EXPECT_THAT(respStr, HasSubstr("/foo"));
    EXPECT_THAT(respStr, HasSubstr("/bar"));
    EXPECT_THAT(respStr, HasSubstr("EchoHandler"));
    EXPECT_THAT(respStr, HasSubstr("/status"));
    EXPECT_THAT(respStr, HasSubstr("StatusHandler"));

    EXPECT_THAT(respStr, HasSubstr("7"));
    EXPECT_THAT(respStr, HasSubstr("13"));

    delete h1;
    delete h2;
    delete s1;
}
