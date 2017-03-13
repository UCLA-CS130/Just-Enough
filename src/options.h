#pragma once
#include <map>
#include <vector>

#include "authenticate.h"
#include "request_handler.h"
#include "config_parser.h"

struct Options {
    unsigned short port;
    unsigned int thread;
    std::map<std::string, RequestHandler*> handlerMap;
    RequestHandler* defaultHandler;
    Authentication* auth;
    bool addHandler(std::string type, std::string path, const NginxConfig& handler_config);
    bool addPath(std::shared_ptr<NginxConfigStatement> handler_config);
    bool addDefaultHandler(std::shared_ptr<NginxConfigStatement> handler_config);
    bool addAuthentication(std::shared_ptr<NginxConfigStatement> auth_config);
    bool addPort(std::shared_ptr<NginxConfigStatement> port_config);
    bool addThread(std::shared_ptr<NginxConfigStatement> thread_config);
    bool loadOptionsFromStream(std::istream* config_file);
    bool loadOptionsFromFile(const char* filename);
    Options();
    ~Options();
};

