#include "gtest/gtest.h"
#include "request_handler.h"

TEST(RequestTest, parseBasic) {
    std::string reqStr = (
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    EXPECT_NE(req, nullptr);

    EXPECT_EQ(req->method(), "GET");
    EXPECT_EQ(req->version(), "HTTP/1.1");
    EXPECT_EQ(req->uri(), "/");
}

TEST(RequestTest, parse_BadVerb) {
    std::string reqStr = (
            "SLAM / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    EXPECT_NE(req, nullptr);

    // TODO: throw an error for bad verbs
    EXPECT_EQ(req->method(), "SLAM");
}

TEST(RequestTest, parse_Malformed) {
    std::string reqStr = (
            "GET / HTTP/1.1 uhh\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    EXPECT_EQ(req, nullptr);
}

TEST(RequestTest, parse_BadHTTP) {
    std::string reqStr = (
            "GET / HTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    EXPECT_EQ(req, nullptr);
}

TEST(RequestTest, parse_BadVersion) {
    std::string reqStr = (
            "GET / HTTP/2.0\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    EXPECT_NE(req, nullptr);

    EXPECT_EQ(req->version(), "HTTP/2.0");
}

TEST(RequestTest, parse_POST) {
    std::string reqStr = (
            "POST / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );
    std::unique_ptr<Request> req = Request::Parse(reqStr);
    EXPECT_NE(req, nullptr);

    EXPECT_EQ(req->method(), "POST");
    EXPECT_EQ(req->uri(), "/");
}

TEST(RequestTest, parse_BadHeaders) {
    std::string reqStr = (
            "POST / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "foo\r\n"
            "\r\n"
            );
    std::unique_ptr<Request> req = Request::Parse(reqStr);
    EXPECT_EQ(req, nullptr);
}

TEST(RequestTest, parse_SomePath) {
    std::string reqStr = (
            "GET /some/path/to/file.jpg HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    EXPECT_NE(req, nullptr);

    EXPECT_EQ(req->method(), "GET");

    EXPECT_EQ(req->uri(), "/some/path/to/file.jpg");
}

TEST(RequestTest, raw_request) {
    std::string reqStr = (
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            );
    std::unique_ptr<Request> req = Request::Parse(reqStr);
    EXPECT_NE(req, nullptr);

    EXPECT_EQ(req->raw_request(), reqStr);
}

TEST(RequestTest, headers) {
    std::string reqStr = (
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "Connection: keep-alive\r\n"
            "\r\n"
            );
    std::unique_ptr<Request> req = Request::Parse(reqStr);
    EXPECT_NE(req, nullptr);

    auto headers = req->headers();

    ASSERT_EQ(headers.size(), 3);
    EXPECT_EQ(headers[0].first, "User-Agent");
    EXPECT_EQ(headers[0].second, "Mozilla/1.0");

    EXPECT_EQ(headers[1].first, "Host");
    EXPECT_EQ(headers[1].second, "localhost:8080");

    EXPECT_EQ(headers[2].first, "Connection");
    EXPECT_EQ(headers[2].second, "keep-alive");
}

TEST(RequestTest, emptybody) {
    std::string reqStr = (
            "POST / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );
    std::unique_ptr<Request> req = Request::Parse(reqStr);
    EXPECT_NE(req, nullptr);

    EXPECT_EQ(req->body(), "");
}

// TODO: implement & test request body
