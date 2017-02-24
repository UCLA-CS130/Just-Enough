#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "echo_handler.h"
#include "status_handler.h"
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
            ws_ = new Webserver(opts_);
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

