CC = g++
CFLAGS = -std=c++11
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lboost_system
DEBUG_FLAGS = -g -Wall -Werror

MAIN = src/main.cc
CC_FILES = $(filter-out $(MAIN), $(wildcard src/*.cc))
TEST_FILES = $(wildcard tests/*.cc)

GTEST_DIR = googletest/googletest
TEST_FLAGS = -isystem ${GTEST_DIR}/include -pthread


all:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(MAIN) $(CC_FILES) -o webserver $(LDFLAGS)

test:
	$(CC) $(CFLAGS) $(TEST_FLAGS) -I$(GTEST_DIR) -c $(GTEST_DIR)/src/gtest-all.cc $(LDFLAGS) 
	$(CC) $(CFLAGS) $(TEST_FLAGS) -Isrc/ $(CC_FILES) $(TEST_FILES) $(GTEST_DIR)/src/gtest_main.cc libgtest.a -o run_tests $(LDFLAGS)
	./run_tests

integration:
	tests/integration.py

