#pragma once
#include "request_handler.h"

class EchoHandler : public RequestHandler {
    public:
        virtual Status Init(const std::string& uri_prefix,
                const NginxConfig& config);

        virtual Status HandleRequest(const Request& request,
                Response* response);
};
