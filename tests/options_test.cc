#include "gtest/gtest.h"
#include "options.h"
#include <sstream>

// PORT TESTS
TEST(OptionsLoadStreamPortTest, BoundaryCases) {
	std::stringstream port0("port 0; thread 1;");
	std::stringstream port1023("port 1023; thread 1;");
	std::stringstream port1024("port 1024; thread 1;");
	std::stringstream port8080("port 8080; thread 1;");
	std::stringstream port65535("port 65535; thread 1;");
	std::stringstream port65545("port 65545; thread 1;");

	Options opt;

	EXPECT_FALSE(opt.loadOptionsFromStream(&port0));
	EXPECT_FALSE(opt.loadOptionsFromStream(&port1023));
	EXPECT_TRUE(opt.loadOptionsFromStream(&port1024));
	EXPECT_TRUE(opt.loadOptionsFromStream(&port8080));
	EXPECT_TRUE(opt.loadOptionsFromStream(&port65535));
	EXPECT_FALSE(opt.loadOptionsFromStream(&port65545));
}

TEST(OptionsLoadStreamPortTest, NoPort) {
	std::stringstream port("path / EchoHandler {}");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&port));
}

TEST(OptionsLoadStreamPortTest, BadNoPort) {
	std::stringstream port("port;");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&port));
}

TEST(OptionsLoadStreamPortTest, PortExtraTokens) {
	std::stringstream port("port 8080 8080;");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&port));
}

TEST(OptionsLoadStreamPortTest, MultiplePorts) {
	std::stringstream port("port 8080; port 54545;");
	
	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&port));
}

TEST(OptionsLoadFileTest, NoFileExists) {
	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromFile("DoesNotExistasasasad"));
}

TEST(OptionsLoadFileTest, ASimpleFile) {
	Options opt;
	EXPECT_TRUE(opt.loadOptionsFromFile("example_config"));
}
//THREADS TESTS
TEST(OptionsLoadStreamThreadTest, badNumThreads) {
	std::stringstream threads("port 8080; threads 0;");
        std::stringstream threads2("port 8080; threads 2001");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&threads));
	EXPECT_FALSE(opt.loadOptionsFromStream(&threads2));
}

//HANDLERS TESTS
TEST(OptionsLoadStreamHandTest, WorkingSimpleCase) {
	std::stringstream mod("port 8080; path /echo EchoHandler {}");
	// TODO: re-enable once StaticHandler is re-implemented
	//std::stringstream mod2("port 8080; path / StaticHandler { root /foo/bar; }");

	Options opt;
	EXPECT_TRUE(opt.loadOptionsFromStream(&mod));
	//EXPECT_TRUE(opt.loadOptionsFromStream(&mod2));
}

TEST(OptionsLoadStreamHandTest, BadStatementSize) {
	std::stringstream mod("port 8080; path /echo {}");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&mod));
}

TEST(OptionsLoadStreamHandTest, NotValidType) {
	std::stringstream mod("port 8080; path / NotARealHandlerType {}");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&mod));
}

TEST(OptionsLoadStreamHandTest, NotValidDefaultStatement) {
	std::stringstream mod("port 8080; default too much {}");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&mod));
}

TEST(OptionsLoadStreamHandTest, NotValidDefaultHandler) {
	std::stringstream mod("port 8080; default NotARealHandlerType {}");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&mod));
}

TEST(OptionsLoadStreamHandTest, DefaultHandlerLoads) {
	std::stringstream mod(
	        "port 8080;"
	        "default EchoHandler {}"
	        );

	Options opt;
	EXPECT_TRUE(opt.loadOptionsFromStream(&mod));

	ASSERT_NE(opt.defaultHandler, nullptr);
	//TODO(evan): add type() method to handlers
	//EXPECT_EQ(opt.defaultHandler->type(), "EchoHandler");
}

TEST(OptionsLoadStreamHandTest, MissingDefault) {
	std::stringstream mod(
	        "port 8080;"
	        );

	Options opt;
	EXPECT_TRUE(opt.loadOptionsFromStream(&mod));

	ASSERT_NE(opt.defaultHandler, nullptr);
	//TODO(evan): add type() method to handlers
	//EXPECT_EQ(opt.defaultHandler->type(), "NotFoundHandler");
}
