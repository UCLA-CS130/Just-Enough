#pragma once

#include "request_handler.h"
#include "request_handler.h"
#include "sync_client.h"

class ProxyHandler : public RequestHandler {
    public:
      ProxyHandler()
      {
        client_ = nullptr;
      }

      virtual Status Init(const std::string& uri_prefix,
              const NginxConfig& config);

      virtual Status HandleRequest(const Request& request,
              Response* response);

      virtual ~ProxyHandler()
      {
        if (client_ != nullptr) {
          delete client_;
        }
      }

      virtual std::string type() const {
          return "ProxyHandler";
      }

    private:
      Response ParseRawResponse(const std::string& response);

      SyncClient *client_;

      std::string uri_prefix_;

      // leave remote port as a string b/c boost needs it in that form anyway
      std::string remote_host_, remote_port_;

      std::string redirect_uri_;
};

REGISTER_REQUEST_HANDLER(ProxyHandler);
