CC = g++
CFLAGS = -std=c++11
UNAME := $(shell uname)
CFLAGS += $(EXTRA_FLAGS)

LDFLAGS =
ifeq ($(UNAME), Linux)
	LDFLAGS += -L/usr/lib/x86_64-linux-gnu -static-libgcc -static-libstdc++
	LDFLAGS += -lpthread -Wl,-Bstatic -lboost_system -lboost_filesystem
	LDFLAGS += -lboost_regex
endif
ifeq ($(UNAME), Darwin) # macOS
	LDFLAGS += -L/usr/local/include -lboost_system -lboost_filesystem
	LDFLAGS += -lboost_regex
endif

MARKDOWN = cpp-markdown-100
CFLAGS += -I$(MARKDOWN)

WARNINGS = -Wall -Wextra -Werror
DEBUG_FLAGS = -g

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
MARKDOWN_OBJ = $(MARKDOWN)/markdown.o $(MARKDOWN)/markdown-tokens.o

GTEST_DIR = googletest/googletest
GMOCK_DIR = googletest/googlemock
TEST_FLAGS = $(DEBUG_FLAGS) -isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include -pthread

HAS_DOCKER := $(shell command -v docker 2> /dev/null)

.PHONY: clean

all: $(OBJ_FILES) $(MARKDOWN_OBJ)
	@echo "$(C)Linking $(CLR)"
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(WARNINGS) $(MAIN) $^ -o webserver $(LDFLAGS)

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

test: $(MARKDOWN_OBJ) $(OBJ_FILES) $(OBJ_DIR)/gtest-all.o $(OBJ_DIR)/gmock-all.o $(TEST_OBJ_FILES)
	@echo "$(C)Linking tests$(CLR)"
	@$(CC) $(CFLAGS) $(TEST_FLAGS) -Isrc/ $^ $(GTEST_DIR)/src/gtest_main.cc -o $(OBJ_DIR)/run_tests $(LDFLAGS)
	@echo "$(C)Running tests$(CLR)"
	@$(OBJ_DIR)/run_tests

$(MARKDOWN)/%.o: $(MARKDOWN)/%.cpp
	@echo "$(C)Compiling Markdown Source: $(CLR) $<"
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

integration:
	tests/integration.py

# Note: coverage requires recompiling everything with new flags
coverage: clean
	@mkdir -p $(COV_DIR)
	@exec make --no-print-directory test EXTRA_FLAGS="$(COVFLAGS)"
	@$(COVCC) $(COVRFLAGS) -s src -o $(OBJ_DIR) $(notdir $(CC_FILES))
	@mv *.gcov $(COV_DIR)/
	@exec make -s clean

test-all:
	$(MAKE) test
	$(MAKE) integration

test-all-clean:
	$(MAKE) clean
	$(MAKE) test
	$(MAKE) integration

deploy:
	@docker ps -q | xargs -L1 docker kill || true # kill running instances
	@docker ps -aq  | xargs docker rm || true # remove container
	@docker build -t webserver.build .
	@docker run webserver.build > binary.tar
	@mkdir -p deployment
	@cd deployment && mkdir -p src
	@cd ..
	@cp -R src deployment/src
	@cp production_config deployment
	@cp -R testFiles1 deployment
	@cp -R testFiles2 deployment
	@cp Dockerfile_shrink deployment/Dockerfile
	@tar -xf binary.tar -C deployment/
	@docker build -t webserver deployment
	@docker ps -aq --filter "ancestor=webserver.build" | xargs docker rm # remove un-shrunk container
	@docker images -q webserver.build | xargs docker rmi # remove un-shrunk image
	@ Note: run shrunk docker container with: docker run --rm -t -p 8080:8080 webserver

EC2_PUBLIC_IP = 54.202.52.105
EC2_PEM = ../my-ec2-key-pair.pem
EC2_USER = ec2-user
EC2_WEBSERVER_DIR = /home/ec2-user
deploy-upload-to-server: deploy
	docker images -q webserver |  xargs -I {} docker save {} > justenough.tar
	scp -i $(EC2_PEM) justenough.tar $(EC2_USER)@$(EC2_PUBLIC_IP):$(EC2_WEBSERVER_DIR)/justenough.tar
	ssh -i $(EC2_PEM) $(EC2_USER)@$(EC2_PUBLIC_IP) "docker ps -aq | xargs -L1 docker kill" || true
	ssh -i $(EC2_PEM) $(EC2_USER)@$(EC2_PUBLIC_IP) "docker ps -aq | xargs -L1 docker rm" || true
	ssh -i $(EC2_PEM) $(EC2_USER)@$(EC2_PUBLIC_IP) "docker images -q | xargs -L1 docker rmi" || true
	ssh -i $(EC2_PEM) $(EC2_USER)@$(EC2_PUBLIC_IP) "docker load < $(EC2_WEBSERVER_DIR)/justenough.tar"
	ssh -i $(EC2_PEM) $(EC2_USER)@$(EC2_PUBLIC_IP) "docker images -q | xargs -I {} docker run -d -t -p 80:8080 {} > /home/ec2-user/log/webserverlog"

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
	@-rm -f $(MARKDOWN)/*.o
	@-rm -f *.tar

clean-docker: clean
ifdef HAS_DOCKER
	@docker ps -q --filter "ancestor=webserver" | xargs docker kill # kill running instances
	@docker ps -aq --filter "ancestor=webserver.build" | xargs docker rm # remove container
	@docker images -q --filter "since=ubuntu:14.04" | xargs -L1 docker rmi # remove images
endif


