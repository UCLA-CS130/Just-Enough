#pragma once
#include "module.h"

class StaticModule : public Module {
    public:
        StaticModule(std::string path, std::string filepath);
        const static std::string typeString;
        static Module* createFromParameters(std::string path, std::shared_ptr<std::map<std::string, std::string>> params);
        virtual bool handleRequest(const HTTPRequest& req, HTTPResponse* resp);
    protected:
        std::string filebase_;
};
