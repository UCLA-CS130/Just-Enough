#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "webserver.h"
#include <sstream>

using ::testing::HasSubstr;

TEST(WebserverTest, processRawRequest) {
    std::string req = (
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "Host: localhost:8080\r\n"
            "\r\n"
            );

    std::string resp = processRawRequest(req);

    EXPECT_THAT(resp, HasSubstr("HTTP/1.1 200 OK\r\n"));
    EXPECT_THAT(resp, HasSubstr("Content-type: text/plain\r\n"));

    // For now, we're expecting it to echo the request
    EXPECT_THAT(resp, HasSubstr(req));
}
