#include "echo_module.h"
#include <iostream>

const std::string EchoModule::typeString = "echo";

Module* EchoModule::createFromParameters(std::string path, std::shared_ptr<std::map<std::string, std::string>> params) {
    Module* mod = new EchoModule(path);
    return mod;
}

bool EchoModule::handleRequest(const HTTPRequest& req, HTTPResponse* resp) {
    resp->okaySetContent(req.getRawRequest(), HTTPContentType_Plain);
    return true;
}
