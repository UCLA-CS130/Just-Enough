#include "gtest/gtest.h"
#include "options.h"
#include <sstream>

// PORT TESTS
TEST(OptionsLoadStreamPortTest, BoundaryCases) {
	std::stringstream port0("server { port = 0;}");
	std::stringstream port1023("server { port = 1023;}");
	std::stringstream port1024("server { port = 1024;}");
	std::stringstream port8080("server { port = 8080;}");
	std::stringstream port65535("server { port = 65535;}");
	std::stringstream port65545("server { port = 65545;}");

	Options opt;

	EXPECT_FALSE(opt.loadOptionsFromStream(&port0));
	EXPECT_FALSE(opt.loadOptionsFromStream(&port1023));
	EXPECT_TRUE(opt.loadOptionsFromStream(&port1024));
	EXPECT_TRUE(opt.loadOptionsFromStream(&port8080));
	EXPECT_TRUE(opt.loadOptionsFromStream(&port65535));
	EXPECT_FALSE(opt.loadOptionsFromStream(&port65545));
}

TEST(OptionsLoadStreamPortTest, NoPort) {
	std::stringstream port("server {}");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&port));
}

TEST(OptionsLoadStreamPortTest, BadNoPort) {
	std::stringstream port("server {port;}");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&port));
}

TEST(OptionsLoadStreamPortTest, BadNoEQ) {
	std::stringstream port("server {port port 8080;}");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&port));
}

TEST(OptionsLoadStreamPortTest, MultiplePorts) {
	std::stringstream port("server { port = 8080; port = 54545; }");
	
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

//MODULES TESTS
TEST(OptionsLoadStreamModTest, WorkingSimpleCase) {
	std::stringstream mod("server { module { type = echo; path = \"/testFiles1\"; }}");

	Options opt;
	EXPECT_TRUE(opt.loadOptionsFromStream(&mod));
}

TEST(OptionsLoadStreamModTest, BadStatementSize) {
	std::stringstream mod("server { module { = echo; } }");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&mod));
}

TEST(OptionsLoadStreamModTest, NoEquals) {
	std::stringstream mod("server { module { type type echo; } }");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&mod));
}

TEST(OptionsLoadStreamModTest, NotValidType) {
	std::stringstream mod("server { module { type = shouldfail; } }");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&mod));
}

//OTHERS
TEST(OptionsLoadStreamTest, NotValidType) {
	std::stringstream mod("server { container { type = shouldfail; } }");

	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&mod));
}

TEST(OptionsLoadModule, QuotedParams) {
	std::stringstream mod("server { module { type = static; path = \"some path\"; filebase = \"testFiles1\";} }");

	Options opt;
	EXPECT_TRUE(opt.loadOptionsFromStream(&mod));
}

