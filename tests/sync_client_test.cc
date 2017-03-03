#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "sync_client.h"

class SyncClientTest : public ::testing::Test {
  public:
    std::string GetRequestStr(const std::string& uri) {
      return "GET " + uri + " HTTP/1.1\r\n" \
        "User-Agent: curl/7.35.0\r\n" \
        "Accept: */*\r\n\r\n";
    }
};

TEST_F(SyncClientTest, IntegratedSanity)
{
  SyncClient client;

  ASSERT_TRUE(client.Connect("ucla.edu", "80"));
  ASSERT_TRUE(client.Write(GetRequestStr("/")));

  std::string response;
  ASSERT_TRUE(client.Read(response));
  ASSERT_TRUE(response.size() > 0);
}
