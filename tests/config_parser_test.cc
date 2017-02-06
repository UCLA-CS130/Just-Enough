#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "config_parser.h"
#include <sstream>

using ::testing::HasSubstr;

TEST(NginxConfigParserTest, SimpleConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;

  bool success = parser.Parse("example_config", &out_config);

  EXPECT_TRUE(success);
}

TEST(NginxConfigParserTest, NonexistantConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;

  bool success = parser.Parse("ThisFileDoesNotExist.config", &out_config);

  EXPECT_FALSE(success);
}

class NginxConfigParserStringTest : public ::testing::Test {
    protected:
        virtual bool parseString(std::string s) {
            std::stringstream test_config_stream(s);

            return parser_.Parse(&test_config_stream, &config_);
        }

        NginxConfigParser parser_;
        NginxConfig config_;
};

TEST_F(NginxConfigParserStringTest, StatementParsed) {
  EXPECT_TRUE(parseString("port 8080;"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // two tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "port");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "8080");
}

TEST_F(NginxConfigParserStringTest, InsignificantWhitespace) {
  EXPECT_TRUE(parseString(" \t   foo  \t   bar     ;    \t"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // two tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "bar");
}

TEST_F(NginxConfigParserStringTest, NoSemicolon) {
  EXPECT_FALSE(parseString("foo bar"));
}

TEST_F(NginxConfigParserStringTest, UnexpectedNewline) {
  EXPECT_TRUE(parseString("\nfoo bar;"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // two tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "bar");
}

TEST_F(NginxConfigParserStringTest, OneLineTwoStatements) {
  EXPECT_TRUE(parseString("foo bar; baz bop;"));

  // two statements
  ASSERT_EQ(config_.statements_.size(), 2);
  // two tokens each
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 2);
  ASSERT_EQ(config_.statements_[1]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "bar");

  EXPECT_EQ(config_.statements_[1]->tokens_[0], "baz");
  EXPECT_EQ(config_.statements_[1]->tokens_[1], "bop");
}

TEST_F(NginxConfigParserStringTest, ChildStatement) {
  EXPECT_TRUE(parseString("foo {\n\tbar baz;\n}"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // one token
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 1);
  // child block has one statement
  ASSERT_EQ(config_.statements_[0]->child_block_->statements_.size(), 1);
  // child block statement has two tokens
  ASSERT_EQ(config_.statements_[0]->child_block_->statements_[0]->tokens_.size(), 2);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->child_block_->statements_[0]->tokens_[0], "bar");
  EXPECT_EQ(config_.statements_[0]->child_block_->statements_[0]->tokens_[1], "baz");
}

TEST_F(NginxConfigParserStringTest, NestedChildStatement) {
  EXPECT_TRUE(parseString(
          "foo {\
            bar {\
              hello;\
            }\
            baz;\
          }"));

  NginxConfigStatement* fooStatement = &*config_.statements_[0];

  NginxConfigStatement* barStatement = &*fooStatement->child_block_->statements_[0];
  NginxConfigStatement* bazStatement = &*fooStatement->child_block_->statements_[1];

  NginxConfigStatement* helloStatement = &*barStatement->child_block_->statements_[0];


  EXPECT_EQ(fooStatement->tokens_[0], "foo");
  EXPECT_EQ(barStatement->tokens_[0], "bar");
  EXPECT_EQ(bazStatement->tokens_[0], "baz");
  EXPECT_EQ(helloStatement->tokens_[0], "hello");
}

TEST_F(NginxConfigParserStringTest, InvalidChildStatement_NoOpenBrace) {
  EXPECT_FALSE(parseString("foo } bar;"));
}

TEST_F(NginxConfigParserStringTest, InvalidChildStatement_NoCloseBrace) {
  EXPECT_FALSE(parseString("foo { bar;"));
}

TEST_F(NginxConfigParserStringTest, ExtraNewline) {
  EXPECT_TRUE(parseString("foo;\n\n\nbar;"));

  // two statements
  ASSERT_EQ(config_.statements_.size(), 2);
  // one tokens each
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 1);
  ASSERT_EQ(config_.statements_[1]->tokens_.size(), 1);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[1]->tokens_[0], "bar");

}

TEST_F(NginxConfigParserStringTest, Comments) {
  EXPECT_TRUE(parseString("foo; # this is a comment ;; } {\nbar;"));

  // two statements
  ASSERT_EQ(config_.statements_.size(), 2);
  // one tokens each
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 1);
  ASSERT_EQ(config_.statements_[1]->tokens_.size(), 1);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[1]->tokens_[0], "bar");

}

TEST_F(NginxConfigParserStringTest, DoubleQuotes) {
  EXPECT_TRUE(parseString("foo = \"Hello, World!\";"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // three tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 3);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "=");
  EXPECT_EQ(config_.statements_[0]->tokens_[2], "\"Hello, World!\"");

}

TEST_F(NginxConfigParserStringTest, DoubleQuotesSpecialChars) {
  EXPECT_TRUE(parseString("foo = \"test \n test; ' } {\";"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // three tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 3);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "=");
  EXPECT_EQ(config_.statements_[0]->tokens_[2], "\"test \n test; ' } {\"");

}

TEST_F(NginxConfigParserStringTest, SingleQuote) {
  EXPECT_TRUE(parseString("foo = '\"';"));

  // one statement
  ASSERT_EQ(config_.statements_.size(), 1);
  // three tokens
  ASSERT_EQ(config_.statements_[0]->tokens_.size(), 3);

  EXPECT_EQ(config_.statements_[0]->tokens_[0], "foo");
  EXPECT_EQ(config_.statements_[0]->tokens_[1], "=");
  EXPECT_EQ(config_.statements_[0]->tokens_[2], "'\"'");
}

TEST_F(NginxConfigParserStringTest, InvalidDoubleQuotes) {
  EXPECT_FALSE(parseString("foo = test\"hello;"));
}

TEST_F(NginxConfigParserStringTest, EmptyConfig) {
  EXPECT_FALSE(parseString(""));
}

TEST_F(NginxConfigParserStringTest, BadStatement) {
  EXPECT_FALSE(parseString(";"));
}

TEST_F(NginxConfigParserStringTest, BadStartBlock) {
  EXPECT_FALSE(parseString("{"));
}

TEST_F(NginxConfigParserStringTest, ToString_basic) {
    std::string inputStr = "port 8080;";
    ASSERT_TRUE(parseString(inputStr));

    std::string s = config_.ToString(0);
    EXPECT_THAT(s, HasSubstr(inputStr));
}

TEST_F(NginxConfigParserStringTest, ToString_block) {
    std::string inputStr = (
            "foo {\n"
            "    bar = baz;\n"
            "}\n"
            ) ;
    ASSERT_TRUE(parseString(inputStr));

    std::string s = config_.ToString(0);
    EXPECT_THAT(s, HasSubstr("bar = baz;"));
    EXPECT_THAT(s, HasSubstr("foo {"));
    EXPECT_THAT(s, HasSubstr("}"));
}

TEST_F(NginxConfigParserStringTest, ToString_block_nested) {
    std::string inputStr = (
            "foo {\n"
            "    bar {\n"
            "        baz = blom;\n"
            "    }\n"
            "}\n"
            ) ;
    ASSERT_TRUE(parseString(inputStr));

    std::string s = config_.ToString(0);
    EXPECT_THAT(s, HasSubstr("baz = blom;"));
    EXPECT_THAT(s, HasSubstr("foo {"));
    EXPECT_THAT(s, HasSubstr("bar {"));
    EXPECT_THAT(s, HasSubstr("}"));
}
