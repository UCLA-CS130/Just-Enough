#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "static_module.h"
#include <map>
#include <memory>
#include <string>

using std::map;
using std::string;
using ::testing::HasSubstr;

TEST(ModuleTest, createStaticModule) {
    auto paramMap = std::make_shared<map<string, string>>(
            std::initializer_list<map<string, string>::value_type>{
            {"type", "static"},
            {"path", "/foo"},
            {"filebase", "testFiles1"},
            });

    std::unique_ptr<Module> mod(createModuleFromParameters(paramMap));
    ASSERT_NE(mod, nullptr);
}

TEST(StaticModuleTest, createStaticModule) {
    auto paramMap = std::make_shared<map<string, string>>(
            std::initializer_list<map<string, string>::value_type>{
            {"type", "static"},
            {"path", "/foo"},
            {"filebase", "testFiles1"},
            });

    std::unique_ptr<Module> mod(StaticModule::createFromParameters("/foo", paramMap));
    ASSERT_NE(mod, nullptr);
}

TEST(StaticModuleTest, needsFilebase) {
    auto paramMap = std::make_shared<map<string, string>>(
            std::initializer_list<map<string, string>::value_type>{
            {"type", "static"},
            {"path", "/foo"},
            });

    std::unique_ptr<Module> mod(StaticModule::createFromParameters("/foo", paramMap));
    ASSERT_EQ(mod, nullptr);
}

TEST(StaticModuleTest, nonexistantFilebase) {
    auto paramMap = std::make_shared<map<string, string>>(
            std::initializer_list<map<string, string>::value_type>{
            {"type", "static"},
            {"path", "/foo"},
            {"filebase", "definitelyNotARealDirectory"},
            });

    std::unique_ptr<Module> mod(StaticModule::createFromParameters("/foo", paramMap));
    ASSERT_EQ(mod, nullptr);
}


class StaticModuleTester : public ::testing::Test {
    protected:
        virtual std::unique_ptr<Module> makeTestStaticModule(std::string filebase) {
            auto paramMap = std::make_shared<map<string, string>>(
                    std::initializer_list<map<string, string>::value_type>{
                    {"type", "echo"},
                    {"path", "/static"},
                    {"filebase", filebase},
                    });

            std::unique_ptr<Module> mod(StaticModule::createFromParameters("/static", paramMap));
            return mod;
        }
};

TEST_F(StaticModuleTester, handleRequest) {
    auto mod = makeTestStaticModule("testFiles1");

    std::string reqStr = (
            "GET /static/page.html HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    ASSERT_NE(req, nullptr);

    Response resp;
    ASSERT_TRUE(mod->handleRequest(*req, &resp));

    string respStr = resp.ToString();
    EXPECT_THAT(respStr, HasSubstr("200 OK"));
    EXPECT_THAT(respStr, HasSubstr("Content-Type: text/html"));
    EXPECT_THAT(respStr, HasSubstr("<html>"));
    EXPECT_THAT(respStr, HasSubstr("</html>"));

    // TODO: use these instead of checking the string response output!
    //EXPECT_EQ(resp.getCode(), HTTPResponseCode_200_OK); // TODO: add getCode() to HTTPResponse
    //EXPECT_EQ(resp.getContent(), reqStr); // TODO: add getContent() to HTTPResponse
}

TEST_F(StaticModuleTester, handleRequestNoFile) {
    auto mod = makeTestStaticModule("testFiles1");

    std::string reqStr = (
            "GET /static/cannot/find/this/file HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    ASSERT_NE(req, nullptr);

    Response resp;
    ASSERT_TRUE(mod->handleRequest(*req, &resp));

    string respStr = resp.ToString();
    EXPECT_THAT(respStr, HasSubstr("404 Not Found"));
}

TEST_F(StaticModuleTester, handleRequestTrailingSlash) {
    auto mod = makeTestStaticModule("testFiles1/");

    std::string reqStr = (
            "GET /static/page.html HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    ASSERT_NE(req, nullptr);

    Response resp;
    ASSERT_TRUE(mod->handleRequest(*req, &resp));

    string respStr = resp.ToString();
    EXPECT_THAT(respStr, HasSubstr("200 OK"));
    EXPECT_THAT(respStr, HasSubstr("Content-Type: text/html"));
    EXPECT_THAT(respStr, HasSubstr("<html>"));
    EXPECT_THAT(respStr, HasSubstr("</html>"));
}

TEST_F(StaticModuleTester, handleRequestImageType) {
    auto mod = makeTestStaticModule("testFiles1/");

    std::string reqStr = (
            "GET /static/cat.gif HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    std::unique_ptr<Request> req = Request::Parse(reqStr);
    ASSERT_NE(req, nullptr);

    Response resp;
    ASSERT_TRUE(mod->handleRequest(*req, &resp));

    string respStr = resp.ToString();
    EXPECT_THAT(respStr, HasSubstr("200 OK"));
    EXPECT_THAT(respStr, HasSubstr("Content-Type: image/gif"));
}
