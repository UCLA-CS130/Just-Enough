#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "echo_handler.h"
#include <map>
#include <memory>
#include <string>

using std::map;
using std::string;
using ::testing::HasSubstr;

TEST(RequestHandlerTest, createEchoHandler) {
    std::unique_ptr<RequestHandler> handler(RequestHandler::CreateByName("EchoHandler"));
    ASSERT_NE(handler, nullptr);

    NginxConfig config;
    RequestHandler::Status status = handler->Init("/foo", config);
    EXPECT_EQ(status, RequestHandler::OK);
}

TEST(EchoHandlerTest, createEchoHandler) {
    NginxConfig config;
    std::unique_ptr<RequestHandler> handler(new EchoHandler());
    ASSERT_NE(handler, nullptr);

    RequestHandler::Status status = handler->Init("/foo", config);
    EXPECT_EQ(status, RequestHandler::OK);
}


class EchoHandlerTester : public ::testing::Test {
    protected:
        virtual std::unique_ptr<RequestHandler> makeTestEchoHandler() {
            NginxConfig config;
            std::unique_ptr<RequestHandler> handler(new EchoHandler());
            handler->Init("/foo", config);
            return handler;
        }
};

TEST_F(EchoHandlerTester, handleRequest) {
    auto handler = makeTestEchoHandler();

    std::string reqStr = (
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    ASSERT_NE(req, nullptr);

    Response resp;
    ASSERT_EQ(handler->HandleRequest(*req, &resp), RequestHandler::OK);

    string respStr = resp.ToString();
    EXPECT_THAT(respStr, HasSubstr("200 OK"));
    EXPECT_THAT(respStr, HasSubstr(reqStr));
}

