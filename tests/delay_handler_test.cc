#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "delay_handler.h"
#include <map>
#include <memory>
#include <string>

using std::map;
using std::string;
using ::testing::HasSubstr;


TEST(RequestHandlerTest, createDelayHandler) {
    std::unique_ptr<RequestHandler> handler(RequestHandler::CreateByName("DelayHandler"));
    ASSERT_NE(handler, nullptr);

    NginxConfig config;
    RequestHandler::Status status = handler->Init("/foo", config);
    EXPECT_EQ(status, RequestHandler::OK);
}

TEST(DelayHandlerTest, createDelayHandler) {
    NginxConfig config;
    std::unique_ptr<RequestHandler> handler(new DelayHandler());
    ASSERT_NE(handler, nullptr);

    RequestHandler::Status status = handler->Init("/foo", config);
    EXPECT_EQ(status, RequestHandler::OK);
}

class DelayHandlerTester : public ::testing::Test {
    protected:
        virtual std::unique_ptr<RequestHandler> makeTestDelayHandler(std::string uri_prefix){
            NginxConfig config;
            std::shared_ptr<NginxConfigStatement> stmt(new NginxConfigStatement());
            stmt->tokens_.push_back ("delay");
            stmt->tokens_.push_back ("1000");
            std::unique_ptr<RequestHandler> handler(new DelayHandler());
            handler->Init(uri_prefix, config);
            return handler;
        }
};

TEST_F(DelayHandlerTester, handleRequest) {
    auto handler = makeTestDelayHandler("/delay");

    std::string reqStr = (
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    ASSERT_NE(req, nullptr);

    Response resp;
    auto start = std::chrono::high_resolution_clock::now();
    ASSERT_EQ(handler->HandleRequest(*req, &resp), RequestHandler::OK);
    auto elapsed = std::chrono::high_resolution_clock::now() - start;

    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    EXPECT_TRUE(microseconds>1000);
    string respStr = resp.ToString();
    EXPECT_THAT(respStr, HasSubstr("200 OK"));
    EXPECT_THAT(respStr, HasSubstr("Delay Test"));
}
