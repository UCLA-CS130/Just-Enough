CC = g++
CFLAGS = -std=c++11
UNAME := $(shell uname)
CFLAGS += $(EXTRA_FLAGS)

LDFLAGS =
ifeq ($(UNAME), Linux)
	LDFLAGS += -L/usr/lib/x86_64-linux-gnu -static-libgcc -static-libstdc++ -lpthread -Wl,-Bstatic -lboost_system -lboost_filesystem
endif
ifeq ($(UNAME), Darwin) # macOS
	LDFLAGS += -L/usr/local/include -lboost_system -lboost_filesystem
endif

DEBUG_FLAGS = -g -Wall -Wextra -Werror

COV_DIR = coverage
COVCC = gcov
COVRFLAGS = -r
COVFLAGS = -fprofile-arcs -ftest-coverage

MAIN = src/main.cc
CC_FILES = $(filter-out $(MAIN), $(wildcard src/*.cc))
TEST_FILES = $(wildcard tests/*.cc)
OBJ_DIR = build
OBJ_FILES = $(addprefix $(OBJ_DIR)/,$(notdir $(CC_FILES:.cc=.o)))
TEST_OBJ_FILES = $(addprefix $(OBJ_DIR)/,$(notdir $(TEST_FILES:.cc=.o)))

GTEST_DIR = googletest/googletest
GMOCK_DIR = googletest/googlemock
TEST_FLAGS = $(DEBUG_FLAGS) -isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include -pthread

all: $(OBJ_FILES)
	@echo "$(C)Linking $(CLR)"
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(MAIN) $^ -o webserver $(LDFLAGS)

$(OBJ_DIR)/%.o: src/%.cc
	@mkdir -p $(OBJ_DIR)
	@echo "$(C)Compiling Source: $(CLR) $<"
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(WARNINGS) -c $< -o $@

$(OBJ_DIR)/%.o: tests/%.cc
	@mkdir -p $(OBJ_DIR)
	@echo "$(C)Compiling Test Source: $(CLR) $<"
	@$(CC) $(CFLAGS) $(TEST_FLAGS) -Isrc/ -I$(GMOCK_DIR) -I$(GTEST_DIR) -c $^ -o $@

$(OBJ_DIR)/gtest-all.o: $(GTEST_DIR)/src/gtest-all.cc
	@mkdir -p $(OBJ_DIR)
	@echo "$(C)Compiling libgtest$(CLR)"
	@$(CC) $(CFLAGS) $(TEST_FLAGS) -I$(GMOCK_DIR) -I$(GTEST_DIR) -c $^ -o $@
$(OBJ_DIR)/gmock-all.o: $(GMOCK_DIR)/src/gmock-all.cc
	@mkdir -p $(OBJ_DIR)
	@echo "$(C)Compiling libgmock$(CLR)"
	@$(CC) $(CFLAGS) $(TEST_FLAGS) -I$(GMOCK_DIR) -I$(GTEST_DIR) -c $^ -o $@

test: $(OBJ_FILES) $(OBJ_DIR)/gtest-all.o $(OBJ_DIR)/gmock-all.o $(TEST_OBJ_FILES)
	@echo "$(C)Linking tests$(CLR)"
	@$(CC) $(CFLAGS) $(TEST_FLAGS) -Isrc/ $^ $(GTEST_DIR)/src/gtest_main.cc -o $(OBJ_DIR)/run_tests $(LDFLAGS)
	@echo "$(C)Running tests$(CLR)"
	@$(OBJ_DIR)/run_tests

integration:
	tests/integration.py

# Note: coverage requires recompiling everything with new flags
coverage: clean
	@mkdir -p $(COV_DIR)
	@exec make --no-print-directory test EXTRA_FLAGS="$(COVFLAGS)"
	@$(COVCC) $(COVRFLAGS) -s src -o $(OBJ_DIR) $(notdir $(CC_FILES))
	@mv *.gcov $(COV_DIR)/
	@exec make -s clean

test-all: test integration

deploy:
	@docker build -t webserver.build .
	@docker run webserver.build > binary.tar
	@mkdir deployment/src
	@cp -R src deployment/src
	@cp example_config deployment
	@cp Dockerfile_shrink deployment/Dockerfile
	@tar -xf binary.tar -C deployment/

clean:
	@-rm -f *.o
	@-rm -f *.a
	@-rm -f *.gcno
	@-rm -f *.gcov
	@-rm -f *.gcda
	@-rm -rf *.dSYM
	@-rm -rf build
	@-rm -f webserver
	@-rm -f run_tests
	@-rm -rf deployment
	@-rm -f *.tar
