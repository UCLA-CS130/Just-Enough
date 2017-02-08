#pragma once
#include "module.h"

class EchoModule : public Module {
    public:
        EchoModule(std::string path) : Module(path) {}
        const static std::string typeString;
        static Module* createFromParameters(std::string path, std::shared_ptr<std::map<std::string, std::string>> params);

        virtual bool handleRequest(const HTTPRequest& req, HTTPResponse* resp);
};
