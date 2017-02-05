#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "http_response.h"

using ::testing::HasSubstr;

TEST(HTTPResponseTest, makeResponseString) {
    HTTPResponse resp;

    resp.okaySetContent("Hello", HTTPContentType_Plain);

    std::string s = resp.makeResponseString();

    EXPECT_THAT(s, HasSubstr("HTTP/1.1 200 OK\r\n"));
    EXPECT_THAT(s, HasSubstr("Content-Type: text/plain\r\n"));
    EXPECT_THAT(s, HasSubstr("Hello"));
}

TEST(HTTPResponseTest, setError) {
    HTTPResponse resp;

    resp.setError(HTTPResponseCode_500_InternalServerError);

    std::string s = resp.makeResponseString();

    EXPECT_THAT(s, HasSubstr("HTTP/1.1 500 Internal Server Error\r\n"));
    EXPECT_THAT(s, HasSubstr("Content-Type:")); // any content type, as long as header exists
    EXPECT_THAT(s, HasSubstr("\r\n\r\n")); // has header end
}

TEST(HTTPResponseTest, setErrorWithContent) {
    HTTPResponse resp;

    resp.setErrorWithContent(HTTPResponseCode_404_NotFound, "Custom 404", HTTPContentType_Plain);

    std::string s = resp.makeResponseString();

    EXPECT_THAT(s, HasSubstr("HTTP/1.1 404 Not Found\r\n"));
    EXPECT_THAT(s, HasSubstr("Content-Type: text/plain"));
    EXPECT_THAT(s, HasSubstr("\r\n\r\nCustom 404"));
}

TEST(HTTPResponseTest, setErrorFromHTTPRequestError) {
    HTTPResponse resp;

    resp.setErrorFromHTTPRequestError(HTTPRequestError_Malformed);

    std::string s = resp.makeResponseString();

    EXPECT_THAT(s, HasSubstr("HTTP/1.1 400 Bad Request\r\n"));
    EXPECT_THAT(s, HasSubstr("Content-Type:")); // any content type, as long as header exists
    EXPECT_THAT(s, HasSubstr("\r\n\r\n")); // has header end
}
