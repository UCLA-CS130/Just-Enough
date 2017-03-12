#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "utils.h"
#include <sstream>

TEST(UtilsTest, split_basic) {
    std::string test = "foo,bar,baz";

    auto v = split(test, ',');

    ASSERT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], "foo");
    EXPECT_EQ(v[1], "bar");
    EXPECT_EQ(v[2], "baz");
}

TEST(UtilsTest, split_trailing) {
    std::string test = "foo,bar,baz,";

    auto v = split(test, ',');

    ASSERT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], "foo");
    EXPECT_EQ(v[1], "bar");
    EXPECT_EQ(v[2], "baz");
}

TEST(UtilsTest, split_empty) {
    std::string test = "";

    auto v = split(test, ',');

    EXPECT_EQ(v.size(), 0);
}

TEST(UtilsTest, getCRLFLine_basic) {
    std::string testStr = (
            "Hello, there!\r\n"
            "How are you?\r\n"
            "\r\n"
            );

    std::stringstream ss(testStr);

    EXPECT_EQ(getCRLFLine(ss), "Hello, there!");
    EXPECT_EQ(getCRLFLine(ss), "How are you?");
    EXPECT_EQ(getCRLFLine(ss), "");
    EXPECT_EQ(getCRLFLine(ss), "");
    EXPECT_EQ(getCRLFLine(ss), "");
}

TEST(UtilsTest, getCRLFLine_noCR) {
    std::string testStr = (
            "Hello, there!\n"
            "How are you?\n"
            "\n"
            );

    std::stringstream ss(testStr);

    EXPECT_EQ(getCRLFLine(ss), "Hello, there!");
    EXPECT_EQ(getCRLFLine(ss), "How are you?");
    EXPECT_EQ(getCRLFLine(ss), "");
    EXPECT_EQ(getCRLFLine(ss), "");
    EXPECT_EQ(getCRLFLine(ss), "");
}

TEST(UtilsTest, mapHasPrefix_contained) {
    std::map<std::string, std::string> m = {
        {"foo", "key1"},
        {"foo/bar", "key2"},
        {"foo/bar/baz", "key3"},
        {"blam", "key4"},
    };

    EXPECT_EQ(mapHasPrefix(m, "nope"), nullptr);
    EXPECT_EQ(mapHasPrefix(m, ""), nullptr);

    EXPECT_EQ( *mapHasPrefix(m, "foo"), "key1");
    EXPECT_EQ( *mapHasPrefix(m, "foo/bar"), "key2");
    EXPECT_EQ( *mapHasPrefix(m, "foo/bar/baz"), "key3");
}

TEST(UtilsTest, mapHasPrefix_prefix) {
    std::map<std::string, std::string> m = {
        {"foo", "key1"},
        {"foo/bar", "key2"},
        {"foo/bar/baz", "key3"},
        {"blam", "key4"},
    };

    EXPECT_EQ(mapHasPrefix(m, "nope"), nullptr);
    EXPECT_EQ(mapHasPrefix(m, ""), nullptr);

    EXPECT_EQ( *mapHasPrefix(m, "foo/hello"), "key1");
    EXPECT_EQ( *mapHasPrefix(m, "foo/bar/world"), "key2");
    EXPECT_EQ( *mapHasPrefix(m, "foo/bar/baz/!"), "key3");
}

TEST(UtilsTest, mapHasPrefix_empty) {
    std::map<std::string, std::string> m = {};

    EXPECT_EQ(mapHasPrefix(m, "nope"), nullptr);
    EXPECT_EQ(mapHasPrefix(m, ""), nullptr);
}

