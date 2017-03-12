#include "gtest/gtest.h"
#include "base64.h"

TEST(Base64Test, encodeDecode) {
    std::string test = "Hello, World!";

    std::string encoded = base64_encode(test);
    std::string decoded = base64_decode(encoded);

    EXPECT_EQ(encoded, "SGVsbG8sIFdvcmxkIQ==");
    EXPECT_EQ(decoded, test);
}

TEST(Base64Test, special_chars) {
    std::string test = "Hello, World! \r\n\0\"'!@#$%^&*()_+[]{}\\|;:,.<>/?";

    std::string encoded = base64_encode(test);
    std::string decoded = base64_decode(encoded);

    EXPECT_EQ(decoded, test);
}
