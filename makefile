CC = g++
CFLAGS = -std=c++0x
DEBUG_FLAGS = -g -Wall

MAIN = src/webserver.cc
CC_FILES = $(filter-out $(MAIN), $(wildcard src/*.cc))
TEST_FILES = $(wildcard tests/*.cc)

GTEST_DIR = googletest/googletest
TEST_FLAGS = -isystem ${GTEST_DIR}/include -pthread


all:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(MAIN) $(CC_FILES) -o webserver

test:
	$(CC) $(CFLAGS) $(TEST_FLAGS) -I$(GTEST_DIR) -c $(GTEST_DIR)/src/gtest-all.cc
	ar -rv libgtest.a gtest-all.o
	$(CC) $(CFLAGS) $(TEST_FLAGS) -Isrc/ $(CC_FILES) $(TEST_FILES) $(GTEST_DIR)/src/gtest_main.cc libgtest.a -o run_tests
	./run_tests

