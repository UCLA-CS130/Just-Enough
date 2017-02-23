#include "gtest/gtest.h"
#include "options.h"
#include <sstream>

// PORT TESTS
TEST(OptionsLoadStreamPortTest, BoundaryCases) {
	std::stringstream port0("port 0;");
	std::stringstream port1023("port 1023;");
	std::stringstream port1024("port 1024;");
	std::stringstream port8080("port 8080;");
	std::stringstream port65535("port 65535;");
	std::stringstream port65545("port 65545;");

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
	EXPECT_TRUE(opt.loadOptionsFromFile("test_config"));
}

//HANDLERS TESTS
/*TEST(OptionsLoadStreamHandTest, WorkingSimpleCase) {
	std::stringstream mod("port 8080; path /echo EchoHandler {}");
	std::stringstream mod2("port 8080; path / StaticHandler { root /foo/bar; }");

	Options opt;
	EXPECT_TRUE(opt.loadOptionsFromStream(&mod));
	EXPECT_TRUE(opt.loadOptionsFromStream(&mod2));
} */

TEST(OptionsLoadStreamHandTest, BadStatementSize) {
	std::stringstream mod("port 8080; path /echo {}");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&mod));
}

TEST(OptionsLoadStreamHandTest, NotValidType) {
	std::stringstream mod("port 8080; module EchoHandler {}");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&mod));
}

