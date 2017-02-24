#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "not_found_handler.h"
#include <map>
#include <memory>
#include <string>

using std::map;
using std::string;
using ::testing::HasSubstr;

TEST(RequestHandlerTest, createNotFoundHandler) {
    std::unique_ptr<RequestHandler> handler(RequestHandler::CreateByName("NotFoundHandler"));
    ASSERT_NE(handler, nullptr);
}

TEST(NotFoundHandlerTest, createNotFoundHandler) {
    NginxConfig config;
    std::unique_ptr<RequestHandler> handler(new NotFoundHandler());
    ASSERT_NE(handler, nullptr);

    RequestHandler::Status status = handler->Init("/", config);
    EXPECT_EQ(status, RequestHandler::OK);
}


class NotFoundHandlerTester : public ::testing::Test {
    protected:
        virtual std::unique_ptr<RequestHandler> makeTestNotFoundHandler() {
            NginxConfig config;
            std::unique_ptr<RequestHandler> handler(new NotFoundHandler());
            handler->Init("/", config);
            return handler;
        }
};

TEST_F(NotFoundHandlerTester, handleRequest) {
    auto handler = makeTestNotFoundHandler();

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
    EXPECT_THAT(respStr, HasSubstr("404 Not Found"));
}

