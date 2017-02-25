#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "static_handler.h"
#include <map>
#include <memory>
#include <string>

using std::map;
using std::string;
using ::testing::HasSubstr;

TEST(RequestHandlerTest, createStaticHandler) {
    std::unique_ptr<RequestHandler> handler(RequestHandler::CreateByName("StaticHandler"));
    ASSERT_NE(handler, nullptr);

    NginxConfig config;
    RequestHandler::Status status = handler->Init("/foo", config);
    EXPECT_EQ(status, RequestHandler::OK);
}

TEST(StaticHandlerTest, createStaticHandler) {
    NginxConfig config;
    std::unique_ptr<RequestHandler> handler(new StaticHandler());
    ASSERT_NE(handler, nullptr);

    RequestHandler::Status status = handler->Init("/foo", config);
    EXPECT_EQ(status, RequestHandler::OK);
}

class StaticHandlerTester : public ::testing::Test {
    protected:
        virtual std::unique_ptr<RequestHandler> makeTestStaticHandler(std::string uri_prefix) {
            NginxConfig config;
            std::shared_ptr<NginxConfigStatement> stmt(new NginxConfigStatement());
            stmt->tokens_.push_back ("root");
            stmt->tokens_.push_back ("testFiles1");
            std::unique_ptr<RequestHandler> handler(new StaticHandler());
            config.statements_.push_back (stmt);
            handler->Init(uri_prefix, config);
            return handler;
        }
};


TEST_F(StaticHandlerTester, handleRequest) {
    auto handler = makeTestStaticHandler("/static");

    std::string reqStr = (
            "GET /static/page.html HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    ASSERT_NE(req, nullptr);

    Response resp;
    ASSERT_EQ(handler->HandleRequest(*req, &resp), RequestHandler::OK);

    string respStr = resp.ToString();
    EXPECT_THAT(respStr, HasSubstr("200 OK"));
    EXPECT_THAT(respStr, HasSubstr("Content-Type: text/html"));
    EXPECT_THAT(respStr, HasSubstr("<html>"));
    EXPECT_THAT(respStr, HasSubstr("</html>"));
}

TEST_F(StaticHandlerTester, handleRequestNoFile) {
    auto handler = makeTestStaticHandler("/static");

    std::string reqStr = (
            "GET /static/cannot/find/this/file HTTP/1.1\r\n"
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

TEST_F(StaticHandlerTester, handleRequestTrailingSlash) {
    auto handler = makeTestStaticHandler("/static");

    std::string reqStr = (
            "GET /static/page.html HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    ASSERT_NE(req, nullptr);

    Response resp;
    ASSERT_EQ(handler->HandleRequest(*req, &resp), RequestHandler::OK);

    string respStr = resp.ToString();
    EXPECT_THAT(respStr, HasSubstr("200 OK"));
    EXPECT_THAT(respStr, HasSubstr("Content-Type: text/html"));
    EXPECT_THAT(respStr, HasSubstr("<html>"));
    EXPECT_THAT(respStr, HasSubstr("</html>"));
}

TEST_F(StaticHandlerTester, handleRequestImageType) {
    auto handler = makeTestStaticHandler("/static");

    std::string reqStr = (
            "GET /static/cat.gif HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    ASSERT_NE(req, nullptr);

    Response resp;
    ASSERT_EQ(handler->HandleRequest(*req, &resp), RequestHandler::OK);

    string respStr = resp.ToString();
    EXPECT_THAT(respStr, HasSubstr("200 OK"));
    EXPECT_THAT(respStr, HasSubstr("Content-Type: image/gif"));
}
