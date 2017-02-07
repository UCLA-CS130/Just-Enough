#include "echo_module.h"
#include <iostream>

const std::string EchoModule::typeString = "echo";

Module* EchoModule::createFromParameters(std::shared_ptr<std::map<std::string, std::string>> params) {
    Module* mod = new EchoModule();
    return mod;
}

bool EchoModule::handleRequest(const HTTPRequest& req, HTTPResponse* resp) {
    return false; // TODO
}
