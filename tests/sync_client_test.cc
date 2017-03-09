#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "sync_client.h"

class SyncClientTest : public ::testing::Test {
  public:
    std::string GetRequestStr(const std::string& host,
        const std::string& uri) {
      return "GET " + uri + " HTTP/1.1\r\n" \
        "User-Agent: curl/7.35.0\r\n" \
        "Accept: */*\r\n" \
        "Host: " + host + "\r\n" \
        "Connection: close\r\n" \
        "\r\n";
    }
};

// The following depends on external hosts and so is tested more thoroughly in integration tests
/*
TEST_F(SyncClientTest, IntegratedSanity)
{
  SyncClient client;
  std::string response;

  // site 1
  ASSERT_TRUE(client.Connect("ucla.edu", "80"));
  ASSERT_TRUE(client.Write(GetRequestStr("ucla.edu", "/")));

  ASSERT_TRUE(client.Read(response));
  ASSERT_TRUE(response.size() > 0);

  // site 2
  ASSERT_TRUE(client.Connect("ipecho.net", "80"));
  ASSERT_TRUE(client.Write(GetRequestStr("ipecho.net", "/plain")));

  ASSERT_TRUE(client.Read(response));
  ASSERT_TRUE(response.size() > 0);
}
*/
