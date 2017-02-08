#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "fileLoader.h"

using ::testing::HasSubstr;

TEST(FileLoaderTest, readsFile) {
    FileLoader fileloader;
    std::string s = "<html><body><h1>hi</h1></body></html>";
    std::stringstream ss(s);

    BinaryData data;
    FileLoaderError err = fileloader.openFile(&ss, "testFile.html", &data);

    ASSERT_EQ(err, FileErr_None);

    EXPECT_EQ(data, s);
    EXPECT_EQ(fileloader.guessMIMEType(), MIMEType_html);
}

TEST(FileLoaderTest, readsJPEGFile) {
    FileLoader fileloader;

    BinaryData data;
    FileLoaderError err = fileloader.openFile("testFiles1/some folder/dog.jpg", &data);

    ASSERT_EQ(err, FileErr_None);

    size_t expectedFileSizeBytes = 141732;
    EXPECT_EQ(data.size(), expectedFileSizeBytes);

    EXPECT_EQ(fileloader.guessMIMEType(), MIMEType_jpeg);
}

TEST(FileLoaderTest, NonexistentFile) {
    FileLoader fileloader;

    BinaryData data;
    FileLoaderError err = fileloader.openFile("ThisFileDoesn'tExist.jpeg", &data);

    ASSERT_NE(err, FileErr_None);
}

TEST(FileLoaderTest, guessMIMETypeGif) {
    FileLoader fileloader;
    BinaryData s = "Some content...";
    std::stringstream ss(s);

    BinaryData data;
    FileLoaderError err = fileloader.openFile(&ss, "testing.gif", &data);

    ASSERT_EQ(err, FileErr_None);

    EXPECT_EQ(fileloader.guessMIMEType(), MIMEType_gif);
}

TEST(FileLoaderTest, guessMIMETypeUnknown) {
    FileLoader fileloader;
    std::string s = "Some content...";
    std::stringstream ss(s);

    BinaryData data;
    FileLoaderError err = fileloader.openFile(&ss, "testing.wat", &data);

    ASSERT_EQ(err, FileErr_None);

    EXPECT_EQ(fileloader.guessMIMEType(), MIMEType_unknown);
}

TEST(FileLoaderTest, guessMIMETypeNoExtention) {
    FileLoader fileloader;
    std::string s = "Some content...";
    std::stringstream ss(s);

    BinaryData data;
    FileLoaderError err = fileloader.openFile(&ss, "thereIsNoExtension", &data);

    ASSERT_EQ(err, FileErr_None);

    EXPECT_EQ(fileloader.guessMIMEType(), MIMEType_unknown);
}
