#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "echo_module.h"
#include <map>
#include <memory>
#include <string>

using std::map;
using std::string;
using ::testing::HasSubstr;

TEST(ModuleTest, createEchoModule) {
    auto paramMap = std::make_shared<map<string, string>>(
            std::initializer_list<map<string, string>::value_type>{
            {"type", "echo"},
            {"path", "/foo"},
            });

    std::unique_ptr<Module> mod(createModuleFromParameters(paramMap));
    ASSERT_NE(mod, nullptr);
}

TEST(EchoModuleTest, createEchoModule) {
    auto paramMap = std::make_shared<map<string, string>>(
            std::initializer_list<map<string, string>::value_type>{
            {"type", "echo"},
            {"path", "/foo"},
            });

    std::unique_ptr<Module> mod(EchoModule::createFromParameters(paramMap));
    ASSERT_NE(mod, nullptr);
}


class EchoModuleTester : public ::testing::Test {
    protected:
        virtual std::unique_ptr<Module> makeTestEchoModule() {
            auto paramMap = std::make_shared<map<string, string>>(
                    std::initializer_list<map<string, string>::value_type>{
                    {"type", "echo"},
                    {"path", "/foo"},
                    });

            std::unique_ptr<Module> mod(EchoModule::createFromParameters(paramMap));
            return mod;
        }
};

TEST_F(EchoModuleTester, handleRequest) {
    auto mod = makeTestEchoModule();

    std::string reqStr = (
            "GET / HTTP/1.1\r\n"
            "User-Agent: Mozilla/1.0\r\n"
            "\r\n"
            );

    HTTPRequest req;
    HTTPRequestError err = req.loadFromRawRequest(reqStr);
    ASSERT_EQ(err, HTTPRequestError_None);

    HTTPResponse resp;
    ASSERT_TRUE(mod->handleRequest(req, &resp));

    string respStr = resp.makeResponseString();
    EXPECT_THAT(respStr, HasSubstr("200 OK"));
    EXPECT_THAT(respStr, HasSubstr(reqStr));

    // TODO: use these instead of checking the string response output!
    //EXPECT_EQ(resp.getCode(), HTTPResponseCode_200_OK); // TODO: add getCode() to HTTPResponse
    //EXPECT_EQ(resp.getContent(), reqStr); // TODO: add getContent() to HTTPResponse
}

