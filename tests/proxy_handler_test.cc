#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "proxy_handler.h"

class ProxyHandlerTest : public ::testing::Test {
  public:
    ProxyHandler* handler_;
    NginxConfig *conf_;
    RequestHandler::Status status_;

    Request GetRequest(const std::string& uri) {
      std::string request_str = \
                                "GET " + uri + " HTTP/1.1\r\n" \
                                "User-Agent: curl/7.35.0\r\n" \
                                "Accept: */*\r\n\r\n";

      auto r = Request::Parse(request_str);
      return *r.get();
    }

    void SetConfig(const std::string& host, const std::string& port)
    {
      if (conf_ != nullptr) {
        delete conf_;
      }
      conf_ = new NginxConfig();
      conf_->statements_.push_back(std::shared_ptr<NginxConfigStatement>(
            new NginxConfigStatement()));
      conf_->statements_[0]->tokens_.push_back("remote_host");
      conf_->statements_[0]->tokens_.push_back(host);
      conf_->statements_.push_back(std::shared_ptr<NginxConfigStatement>(
            new NginxConfigStatement()));
      conf_->statements_[1]->tokens_.push_back("remote_port");
      conf_->statements_[1]->tokens_.push_back(port);
    }

    void SetUp() {
      handler_ = new ProxyHandler();
      conf_ = nullptr;
    }

    void TearDown() {
      delete handler_;
      if (conf_ != nullptr) {
        delete conf_;
      }
    }
};

TEST_F(ProxyHandlerTest, ResponseParsingTest)
{
  Response r = handler_->ParseRawResponse("HTTP/1.1 200 OK\r\n\r\n");
  ASSERT_EQ(r.status(), Response::code_200_OK);
  ASSERT_EQ(r.headers().size(), 0);
  ASSERT_EQ(r.body().size(), 0);

  r = handler_->ParseRawResponse("HTTP/1.1 302 FOUND\r\nHost: google.com\r\n\r\n");
  ASSERT_EQ(r.status(), Response::code_302_found);
  ASSERT_EQ(r.headers().size(), 1);
  ASSERT_EQ(r.headers()[0].first, "Host");
  ASSERT_EQ(r.headers()[0].second, "google.com");
  ASSERT_EQ(r.body().size(), 0);

  r = handler_->ParseRawResponse("HTTP/1.1 200 OK\r\nHost: google.com\r\n\r\nhello, world!");
  ASSERT_EQ(r.status(), Response::code_200_OK);
  ASSERT_EQ(r.headers().size(), 1);
  ASSERT_EQ(r.headers()[0].first, "Host");
  ASSERT_EQ(r.headers()[0].second, "google.com");
  ASSERT_STREQ(r.body().c_str(), "hello, world!");
}

/*
 * The following tests require dependency on external hosts ucla.edu, ipecho.ent, google.com
 * Since not all testing environments (e.g. travis) will have access to such hosts
 * these tests are currently disabled, and are covered more thoroughly in integration tests
 */
/*
TEST_F(ProxyHandlerTest, SanityTest)
{
  Response r;
  // proxy to ipecho.net/plain
  SetConfig("ipecho.net", "80");
  handler_->Init("/proxy", *conf_);
  status_ = handler_->HandleRequest(GetRequest("/proxy/plain"), &r);
  ASSERT_EQ(status_, RequestHandler::OK);
  ASSERT_EQ(r.status(), Response::code_200_OK);
}

TEST_F(ProxyHandlerTest, MovedTest)
{
  Response r;
  // 301 redirect
  SetConfig("google.com", "80");
  handler_->Init("/proxy", *conf_);
  status_ = handler_->HandleRequest(GetRequest("/proxy"), &r);
  ASSERT_EQ(status_, RequestHandler::OK);
  ASSERT_EQ(r.status(), Response::code_200_OK);

}

TEST_F(ProxyHandlerTest, FoundTest) {
  Response r;
  // 302 redirect
  SetConfig("ucla.edu", "80");
  handler_->Init("/proxy", *conf_);
  status_ = handler_->HandleRequest(GetRequest("/proxy"), &r);
  ASSERT_EQ(status_, RequestHandler::OK);
  ASSERT_EQ(r.status(), Response::code_200_OK);
}

TEST_F(ProxyHandlerTest, NotFoundTest)
{
  Response r;
  SetConfig("ipecho.net", "80");
  handler_->Init("/proxy", *conf_);
  status_ = handler_->HandleRequest(GetRequest("/proxy/asdf"), &r);
  ASSERT_EQ(status_, RequestHandler::OK);
  ASSERT_EQ(r.status(), Response::code_404_not_found);
}
*/
