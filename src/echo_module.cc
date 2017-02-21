#include "echo_module.h"
#include <iostream>

const std::string EchoModule::typeString = "echo";

Module* EchoModule::createFromParameters(std::string path, std::shared_ptr<std::map<std::string, std::string>> params) {
    Module* mod = new EchoModule(path);
    return mod;
}

bool EchoModule::handleRequest(const Request& req, Response* resp) {
    resp->SetStatus(Response::code_200_OK);
    resp->SetBody(req.raw_request());
    resp->AddHeader("Content-Type", "text/plain");
    return true;
}
