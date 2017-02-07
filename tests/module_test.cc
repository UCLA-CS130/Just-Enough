#include "gtest/gtest.h"
#include "module.h"
#include "echo_module.h"
#include <map>
#include <memory>
#include <string>

using std::map;
using std::string;

TEST(ModuleTest, createModuleDestructor) {
    auto paramMap = std::make_shared<map<string, string>>(
            std::initializer_list<map<string, string>::value_type>{
            {"type", "echo"},
            {"path", "/foo"},
            });

    Module* mod = createModuleFromParameters(paramMap);
    ASSERT_NE(mod, nullptr);
    delete mod;
}

TEST(ModuleTest, createModule_badType) {
    auto paramMap = std::make_shared<map<string, string>>(
            std::initializer_list<map<string, string>::value_type>{
            {"type", "definitelyNotARealModuleType"},
            {"path", "/foo"},
            });

    Module* mod = createModuleFromParameters(paramMap);
    ASSERT_EQ(mod, nullptr);
}

TEST(ModuleTest, createModule_noType) {
    auto paramMap = std::make_shared<map<string, string>>(
            std::initializer_list<map<string, string>::value_type>{
            {"path", "/foo"},
            });

    Module* mod = createModuleFromParameters(paramMap);
    ASSERT_EQ(mod, nullptr);
}

TEST(ModuleTest, createModule_noPath) {
    auto paramMap = std::make_shared<map<string, string>>(
            std::initializer_list<map<string, string>::value_type>{
            {"type", "definitelyNotARealModuleType"},
            });

    Module* mod = createModuleFromParameters(paramMap);
    ASSERT_EQ(mod, nullptr);
}

TEST(ModuleTest, matchesRequestPath) {
    Module* mod = EchoModule::createFromParameters("/foo/bar", nullptr);
    ASSERT_NE(mod, nullptr);

    EXPECT_TRUE(mod->matchesRequestPath("/foo/bar"));
    EXPECT_TRUE(mod->matchesRequestPath("/foo/bar/"));
    EXPECT_TRUE(mod->matchesRequestPath("/foo/bar/baz.png"));
    EXPECT_TRUE(mod->matchesRequestPath("/foo/bar/flom/baz.png"));

    EXPECT_FALSE(mod->matchesRequestPath("/bad/"));
    EXPECT_FALSE(mod->matchesRequestPath("/foo/"));
    EXPECT_FALSE(mod->matchesRequestPath("/foo/bad"));
    EXPECT_FALSE(mod->matchesRequestPath("/foo/bad/baz"));

    EXPECT_TRUE(mod->matchesRequestPath("/foo/barnyard"));
}

TEST(ModuleTest, matchesRequestPathTrailingSlash) {
    Module* mod = EchoModule::createFromParameters("/foo/bar/", nullptr);
    ASSERT_NE(mod, nullptr);

    EXPECT_TRUE(mod->matchesRequestPath("/foo/bar"));
    EXPECT_TRUE(mod->matchesRequestPath("/foo/bar/"));
    EXPECT_TRUE(mod->matchesRequestPath("/foo/bar/baz.png"));
    EXPECT_TRUE(mod->matchesRequestPath("/foo/bar/flom/baz.png"));

    EXPECT_FALSE(mod->matchesRequestPath("/bad/"));
    EXPECT_FALSE(mod->matchesRequestPath("/foo/"));
    EXPECT_FALSE(mod->matchesRequestPath("/foo/bad"));
    EXPECT_FALSE(mod->matchesRequestPath("/foo/bad/"));
    EXPECT_FALSE(mod->matchesRequestPath("/foo/bad/baz"));

    EXPECT_FALSE(mod->matchesRequestPath("/foo/barnyard"));
}

TEST(ModuleTest, matchesRequestPathCatchall) {
    Module* mod = EchoModule::createFromParameters("/", nullptr);
    ASSERT_NE(mod, nullptr);

    EXPECT_TRUE(mod->matchesRequestPath("/"));
    EXPECT_TRUE(mod->matchesRequestPath("/foo"));
    EXPECT_TRUE(mod->matchesRequestPath("/foo.bar"));
    EXPECT_TRUE(mod->matchesRequestPath("/foo/bar"));
}

