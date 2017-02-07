#include "gtest/gtest.h"
#include "module.h"
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
