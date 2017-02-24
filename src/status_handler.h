#pragma once
#include "request_handler.h"

class StatusHandler : public RequestHandler {
    public:
        virtual Status Init(const std::string& uri_prefix,
                const NginxConfig& config);

        virtual Status HandleRequest(const Request& request,
                Response* response);

        virtual ~StatusHandler() = default;

        virtual std::string type() const {
            return "StatusHandler";
        }
};

REGISTER_REQUEST_HANDLER(StatusHandler);
