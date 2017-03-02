#pragma once
#include "request_handler.h"
const size_t DEFAULT_DELAY = 1000;

//Used for testing multi-threading only
class DelayHandler: public RequestHandler{
  public:
    virtual Status Init(const std::string& uri_prefix,
            const NginxConfig& config);

    virtual Status HandleRequest(const Request& request,
            Response* response);

    virtual ~DelayHandler() = default;

    virtual std::string type() const {
        return "DelayHandler";
    }
  private:
    int delay_;
};

REGISTER_REQUEST_HANDLER(DelayHandler);
