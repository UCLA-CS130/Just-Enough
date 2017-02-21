#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "request_handler.h"

using ::testing::HasSubstr;

TEST(ResponseTest, basicToString) {
    Response resp;

    std::string body = "Hello.";
    resp.SetStatus(Response::code_200_OK);
    resp.SetBody(body);

    std::string s = resp.ToString();

    EXPECT_THAT(s, HasSubstr("HTTP/1.1 200 OK\r\n"));
    EXPECT_THAT(s, HasSubstr(body));
}

TEST(ResponseTest, addHeader) {
    Response resp;

    std::string body = "Hello.";
    resp.SetStatus(Response::code_200_OK);
    resp.SetBody(body);
    resp.AddHeader("Content-Type", "text/plain");
    resp.AddHeader("Connection", "closed");

    std::string s = resp.ToString();

    EXPECT_THAT(s, HasSubstr("HTTP/1.1 200 OK\r\n"));
    EXPECT_THAT(s, HasSubstr(body));
    EXPECT_THAT(s, HasSubstr("Content-Type: text/plain\r\n"));
    EXPECT_THAT(s, HasSubstr("Connection: closed\r\n"));
}

TEST(ResponseTest, non_200_response) {
    Response resp;

    std::string body = "Hello.";
    resp.SetStatus(Response::code_404_not_found);
    resp.SetBody(body);

    std::string s = resp.ToString();

    EXPECT_THAT(s, HasSubstr("HTTP/1.1 404 Not Found\r\n"));
    EXPECT_THAT(s, HasSubstr(body));
}
