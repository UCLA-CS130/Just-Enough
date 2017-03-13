#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "markdown_to_html.h"
#include <string>

using ::testing::HasSubstr;

TEST(MarkdownToHTMLTest, SimpleCase) {
	std::string markdownString("*not*");

	EXPECT_THAT(convertMarkdownToHtml(markdownString), HasSubstr("<em>not</em>"));
}