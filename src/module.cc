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
        Module* mod = EchoModule::createFromParameters(pathParam->second, params);
        return mod;
    } else if (typeParam->second == StaticModule::typeString) {
        Module* mod = StaticModule::createFromParameters(params);
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

/* determine if a request's path matches this Module's configured path.
 * path format is purely prefixed based, with an exception for trailing '/':
 *   "/foo" would match "/foo", "/foo/bar", AND "/foobar"
 *      translates to regex: "/foo.*"
 *   "/foo/" would match "/foo", "/foo/bar", but NOT "/foobar"
 *      translates to regex: "/foo/?.*"
 * This is because "/foo" and "/foo/" should act identically in terms of file requests.
 * Module subclasses can override this method if they have other considerations.
 */
bool Module::matchesRequestPath(const std::string& reqPath) const {
    if (reqPath.size() < path_.size() - 1) return false;

    size_t pathSize = path_.size();
    if ((reqPath.size() == path_.size() - 1)
            && (path_.size() > 1) && (path_[path_.size()-1] == '/')) {
        pathSize--;
    }

    for (int i = 0; i < pathSize; ++i) {
        if (path_[i] != reqPath[i]) {
            return false;
        }
    }

    return true;
}
