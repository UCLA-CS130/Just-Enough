#include <string>
#include <map>
#include <iostream>

#include "module.h"
#include "echo_module.h"
#include "static_module.h"

Module* createModuleFromParameters(std::shared_ptr<std::map<std::string, std::string>> params) {
    auto typeParam = params->find("type");
    if (typeParam == params->end()) {
        std::cerr << "Must specify module type." << std::endl;
        return nullptr;
    }

    auto pathParam = params->find("path");
    if (pathParam == params->end()) {
        std::cerr << "Must specify path for module to handle." << std::endl;
        return nullptr;
    }

    if (typeParam->second == EchoModule::typeString) {
        Module* mod = EchoModule::createFromParameters(params);
        return mod;
    } else if (typeParam->second == StaticModule::typeString) {
        Module* mod = StaticModule::createFromParameters(params);
        return mod;
    } else {
        std::cerr << "Unknown module type \"" << typeParam->second << "\"." << std::endl;
        return nullptr;
    }
    return nullptr;
}

bool Module::matchesRequestPath(const std::string& str) const {
    return false; //TODO
}
