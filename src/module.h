#pragma once
#include "http_request.h"
#include "http_response.h"

#include <map>
#include <memory>
#include <string>

class Module {
    public:
        virtual ~Module() = default;
        virtual bool handleRequest(const HTTPRequest& req, HTTPResponse* resp) = 0;
        virtual bool matchesRequestPath(const std::string& str) const;

    private:
        std::shared_ptr<std::map<std::string, std::string>> moduleParameters;
};

Module* createModuleFromParameters(std::shared_ptr<std::map<std::string, std::string>> params);
