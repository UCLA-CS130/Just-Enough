#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "authenticate.h"
#include "base64.h"

using std::string;

using ::testing::HasSubstr;

class AuthenticationTest : public ::testing::Test {
    protected:
        // given an array of path prefix to RequestHandler*, initialize a webserver instance
        virtual void createAuthentication() {
            auth_ = std::unique_ptr<Authentication>(new Authentication());
            auth_->addRealm("/secret", new AuthenticationRealm({{"user1", "pass1"}, {"user2", "password2"}}));
        }

        std::unique_ptr<Authentication> auth_;
};

TEST_F(AuthenticationTest, requiresAuth) {
    createAuthentication();

    auto reqSecret = Request::Parse(
            "GET /secret HTTP/1.1\r\n"
            "\r\n"
            );
    auto reqPublic = Request::Parse(
            "GET /public HTTP/1.1\r\n"
            "\r\n"
            );
    auto reqSecretSub = Request::Parse(
            "GET /secret/file.txt HTTP/1.1\r\n"
            "\r\n"
            );

    EXPECT_TRUE(auth_->requestRequiresAuthentication(*reqSecret));
    EXPECT_FALSE(auth_->requestRequiresAuthentication(*reqPublic));
    EXPECT_TRUE(auth_->requestRequiresAuthentication(*reqSecretSub));
}

TEST_F(AuthenticationTest, requestPassesAuthentication) {
    createAuthentication();

    auto reqGood1 = Request::Parse(
            (string("GET /secret HTTP/1.1\r\n"
            "Authorization: Basic ") + base64_encode("user1:pass1") + string("\r\n"
            "\r\n")).c_str()
            );
    auto reqGood2 = Request::Parse(
            (string("GET /secret HTTP/1.1\r\n"
            "Authorization: Basic ") + base64_encode("user2:password2") + string("\r\n"
            "\r\n")).c_str()
            );
    auto reqGoodSub = Request::Parse(
            (string("GET /secret/file.txt HTTP/1.1\r\n"
            "Authorization: Basic ") + base64_encode("user1:pass1") + string("\r\n"
            "\r\n")).c_str()
            );

    auto reqBadUser = Request::Parse(
            (string("GET /secret HTTP/1.1\r\n"
            "Authorization: Basic ") + base64_encode("person:pass1") + string("\r\n"
            "\r\n")).c_str()
            );
    auto reqBadPass = Request::Parse(
            (string("GET /secret HTTP/1.1\r\n"
            "Authorization: Basic ") + base64_encode("user1:foobar") + string("\r\n"
            "\r\n")).c_str()
            );
    auto reqBadMix = Request::Parse(
            (string("GET /secret HTTP/1.1\r\n"
            "Authorization: Basic ") + base64_encode("user1:password2") + string("\r\n"
            "\r\n")).c_str()
            );

    auto reqBadMissing = Request::Parse(
            "GET /secret HTTP/1.1\r\n"
            "\r\n"
            );
    auto reqBadUnknownScheme = Request::Parse(
            (string("GET /secret HTTP/1.1\r\n"
            "Authorization: Complicated ") + base64_encode("user1:pass1") + string("\r\n"
            "\r\n")).c_str()
            );
    auto reqBadNoBase64 = Request::Parse(
            (string("GET /secret HTTP/1.1\r\n"
            "Authorization: Basic ") + "user1:pass1" + string("\r\n"
            "\r\n")).c_str()
            );
    auto reqBadNoScheme = Request::Parse(
            (string("GET /secret HTTP/1.1\r\n"
            "Authorization: ") + base64_encode("user1:pass1") + string("\r\n"
            "\r\n")).c_str()
            );

    ASSERT_NE(reqGood1, nullptr);
    ASSERT_NE(reqGood2, nullptr);
    ASSERT_NE(reqGoodSub, nullptr);

    ASSERT_NE(reqBadUser, nullptr);
    ASSERT_NE(reqBadPass, nullptr);
    ASSERT_NE(reqBadMix, nullptr);

    ASSERT_NE(reqBadMissing, nullptr);
    ASSERT_NE(reqBadUnknownScheme, nullptr);
    ASSERT_NE(reqBadNoBase64, nullptr);
    ASSERT_NE(reqBadNoScheme, nullptr);

    EXPECT_TRUE(auth_->requestPassesAuthentication(*reqGood1));
    EXPECT_TRUE(auth_->requestPassesAuthentication(*reqGood2));
    EXPECT_TRUE(auth_->requestPassesAuthentication(*reqGoodSub));

    EXPECT_FALSE(auth_->requestPassesAuthentication(*reqBadUser));
    EXPECT_FALSE(auth_->requestPassesAuthentication(*reqBadPass));
    EXPECT_FALSE(auth_->requestPassesAuthentication(*reqBadMix));

    EXPECT_FALSE(auth_->requestPassesAuthentication(*reqBadMissing));
    EXPECT_FALSE(auth_->requestPassesAuthentication(*reqBadUnknownScheme));
    EXPECT_FALSE(auth_->requestPassesAuthentication(*reqBadNoBase64));
    EXPECT_FALSE(auth_->requestPassesAuthentication(*reqBadNoScheme));
}


TEST_F(AuthenticationTest, generateFailedAuthenticationResponse) {
    createAuthentication();

    auto req = Request::Parse(
            "GET /secret HTTP/1.1\r\n"
            "\r\n"
            );
    ASSERT_NE(req, nullptr);

    Response resp;
    auth_->generateFailedAuthenticationResponse(*req, &resp);

    string respStr = resp.ToString();
    EXPECT_THAT(respStr, HasSubstr("401 Unauthorized"));
    EXPECT_THAT(respStr, HasSubstr("WWW-Authenticate: Basic realm=\"/secret\"\r\n"));
}
