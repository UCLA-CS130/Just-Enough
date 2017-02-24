#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "multimap_counter.h"
#include <sstream>
#include <thread>

TEST(MultimapCounterTest, basic) {
    MultiMapCounter<std::string, int> mm_counter;

    mm_counter.increment("foo", 1);
    mm_counter.increment("foo", 1);
    mm_counter.increment("foo", 1);

    mm_counter.increment("foo", 2);
    mm_counter.increment("foo", 2);

    mm_counter.increment("bar", 1);
    mm_counter.increment("bar", 2);
    mm_counter.increment("bar", 3);

    mm_counter.increment("baz", 5);

    EXPECT_EQ(mm_counter.get("foo", 1), 3);
    EXPECT_EQ(mm_counter.get("foo", 2), 2);
    EXPECT_EQ(mm_counter.get("foo", 3), 0);

    EXPECT_EQ(mm_counter.get("bar", 1), 1);
    EXPECT_EQ(mm_counter.get("bar", 2), 1);
    EXPECT_EQ(mm_counter.get("bar", 3), 1);

    EXPECT_EQ(mm_counter.get("baz", 1), 0);
    EXPECT_EQ(mm_counter.get("baz", 5), 1);
}

TEST(MultimapCounterTest, largerIncrement) {
    MultiMapCounter<std::string, int> mm_counter;

    mm_counter.increment("foo", 1, 5);
    mm_counter.increment("foo", 1);
    mm_counter.increment("foo", 1);

    mm_counter.increment("bar", 4, 100);

    EXPECT_EQ(mm_counter.get("foo", 1), 7);
    EXPECT_EQ(mm_counter.get("foo", 2), 0);

    EXPECT_EQ(mm_counter.get("bar", 4), 100);
}

TEST(MultimapCounterTest, iterateInner) {
    MultiMapCounter<std::string, int> mm_counter;

    mm_counter.increment("foo", 1);
    mm_counter.increment("foo", 1);
    mm_counter.increment("foo", 2);
    mm_counter.increment("foo", 3);

    for (auto p : mm_counter.iterateKey("foo")) {
        switch (p.first) {
            case 1:
                EXPECT_EQ(p.second, 2);
                break;
            case 2:
                EXPECT_EQ(p.second, 1);
                break;
            case 3:
                EXPECT_EQ(p.second, 1);
                break;
            default:
                EXPECT_TRUE(false);
                break;
        }
    }
}

TEST(MultimapCounterTest, getKeys) {
    MultiMapCounter<std::string, int> mm_counter;

    mm_counter.increment("foo", 1);
    mm_counter.increment("foo", 2);
    mm_counter.increment("bar", 3);
    mm_counter.increment("baz", 2);

    auto vec = mm_counter.getKeys();
    ASSERT_EQ(vec.size(), 3);
    EXPECT_TRUE(vec[0] == "foo" || vec[1] == "foo" || vec[2] == "foo");
    EXPECT_TRUE(vec[0] == "bar" || vec[1] == "bar" || vec[2] == "bar");
    EXPECT_TRUE(vec[0] == "baz" || vec[1] == "baz" || vec[2] == "baz");
    EXPECT_TRUE(vec[0] != vec[1] && vec[1] != vec[2] && vec[0] != vec[2]);
}

TEST(MultimapCounterTest, theadedTest) {
    MultiMapCounter<std::string, int> mm_counter;

    int n = 100;

    // run several threads accessing same map
    std::thread t1([&]() {
            for (int i = 0; i < n; i++) {
                mm_counter.increment("foo", 1);
            }
            });
    std::thread t2([&]() {
            for (int i = 0; i < n; i++) {
                mm_counter.increment("foo", 1);
            }
            });
    std::thread t3([&]() {
            for (int i = 0; i < n; i++) {
                mm_counter.increment("foo", 1);
            }
            });
    std::thread t4([&]() {
            for (int i = 0; i < n; i++) {
                mm_counter.increment("bar", 5);
            }
            });
    std::thread t5([&]() {
            for (int i = 0; i < n; i++) {
                mm_counter.increment("bar", 5);
            }
            });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    for (auto p : mm_counter.iterateKey("foo")) {
        switch (p.first) {
            case 1:
                EXPECT_EQ(p.second, 3*n*1);
                break;
            case 2:
                EXPECT_EQ(p.second, 2*n*5);
                break;
            default:
                EXPECT_TRUE(false);
                break;
        }
    }
}
