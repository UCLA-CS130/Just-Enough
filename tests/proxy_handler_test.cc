#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "proxy_handler.h"

class ProxyHandlerTest : public ::testing::Test {
public:
	ProxyHandler* handler_;
  NginxConfig conf_;
  NginxConfigParser parser_;

	Request GetRequest(const std::string& uri) {
		std::string request_str = \
			"GET " + uri + " HTTP/1.1\r\n" \
			"User-Agent: curl/7.35.0\r\n" \
      "Accept: */*\r\n\r\n";

		auto r = Request::Parse(request_str);
		return *r.get();
	}

  ProxyHandlerTest()
  {
    std::string config_str = \
      "remote_host google.com;\n" \
      "remote_port 80;\n";
    std::istringstream config_stream(config_str);
    if (!parser_.Parse(&config_stream, &conf_)) {
      ADD_FAILURE() << "Failed to parse configuration:\n.";
    }
  }

  void SetUp() {
    handler_ = new ProxyHandler();
  }

	void TearDown() {
		if (handler_ != nullptr)
			delete handler_;
      handler_ = nullptr;
	}
};

TEST_F(ProxyHandlerTest, SanityTest)
{
  handler_->Init("/proxy", conf_);
  Response response;
  RequestHandler::Status status =
    handler_->HandleRequest(GetRequest("/proxy"), &response);
  ASSERT_EQ(status, RequestHandler::OK);
}
