#pragma once
#include "request_handler.h"

class StaticHandler : public RequestHandler {
    public:
        virtual Status Init(const std::string& uri_prefix,
              const NginxConfig& config);

        virtual Status HandleRequest(const Request& request,
              Response* response);

        virtual ~StaticHandler();

        virtual std::string type() const {
            return "StaticHandler";
        }


    protected:
        std::string filebase_;
        bool convert_markdown_;
        std::string path_;
        RequestHandler* notFoundHandler_;
};

REGISTER_REQUEST_HANDLER(StaticHandler);
