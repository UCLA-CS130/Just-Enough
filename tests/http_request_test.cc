#include "gtest/gtest.h"
#include "http_request.h"

TEST(HTTPRequestTest, loadFromRawRequestBasic) {
    std::string reqStr = (
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            );

    HTTPRequest req;
    HTTPRequestError err = req.loadFromRawRequest(reqStr);
    EXPECT_EQ(err, HTTPRequestError_None);

    EXPECT_EQ(req.getVerb(), HTTPVerb_GET);

    EXPECT_EQ(req.getPath(), "/");

    EXPECT_EQ(req.getHeader("User-Agent"), "Mozilla/1.0");
    EXPECT_EQ(req.getHeader("Connection"), "keep-alive");
}

TEST(HTTPRequestTest, loadFromRawRequest_BadVerb) {
    std::string reqStr = (
            "SLAM / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    HTTPRequest req;
    HTTPRequestError err = req.loadFromRawRequest(reqStr);
    EXPECT_EQ(err, HTTPRequestError_BadVerb);
}

TEST(HTTPRequestTest, loadFromRawRequest_Malformed) {
    std::string reqStr = (
            "GET / HTTP/1.1 uhh\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    HTTPRequest req;
    HTTPRequestError err = req.loadFromRawRequest(reqStr);
    EXPECT_EQ(err, HTTPRequestError_Malformed);
}

TEST(HTTPRequestTest, loadFromRawRequest_SomePath) {
    std::string reqStr = (
            "GET /some/path/to/file.jpg HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            );

    HTTPRequest req;
    HTTPRequestError err = req.loadFromRawRequest(reqStr);
    EXPECT_EQ(err, HTTPRequestError_None);

    EXPECT_EQ(req.getVerb(), HTTPVerb_GET);

    EXPECT_EQ(req.getPath(), "/some/path/to/file.jpg");

    EXPECT_EQ(req.getHeader("User-Agent"), "Mozilla/1.0");
    EXPECT_EQ(req.getHeader("Connection"), "keep-alive");
}
