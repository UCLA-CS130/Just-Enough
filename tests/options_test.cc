#include "gtest/gtest.h"
#include "options.h"
#include <sstream>

TEST(OptionsLoadStreamPortTest, BoundaryCases) {
	std::stringstream port0("server { listen 0;}");
	std::stringstream port1023("server { listen 1023;}");
	std::stringstream port1024("server { listen 1024;}");
	std::stringstream port8080("server { listen 8080;}");
	std::stringstream port65535("server { listen 65535;}");
	std::stringstream port65545("server { listen 65545;}");

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

TEST(OptionsLoadStreamPortTest, MultiplePorts) {
	std::stringstream port("server { listen 8080; listen 54545; }");
	
	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromStream(&port));
}

TEST(OptionsLoadFileTest, NoFileExists) {
	Options opt;
	EXPECT_FALSE(opt.loadOptionsFromFile("DoesNotExistasasasad"));
}

//Test for simple existing file