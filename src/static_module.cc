#include "static_module.h"
#include "fileLoader.h"
#include <iostream>
#include <boost/filesystem.hpp>

const std::string StaticModule::typeString = "static";

Module* StaticModule::createFromParameters(std::shared_ptr<std::map<std::string, std::string>> params) {
    auto filebaseParam = params->find("filebase");
    if (filebaseParam == params->end()) {
        std::cerr << "Static Module must specify 'filebase' parameter." << std::endl;
        return nullptr;
    }

    if ( ! boost::filesystem::exists(filebaseParam->second)) {
        std::cerr << "Invalid 'filebase' parameter for Static Module: directory not found." << std::endl;
        return nullptr;
    }

    Module* mod = new StaticModule(filebaseParam->second);
    return mod;
}

StaticModule::StaticModule(std::string filebase)
    : filebase_(filebase)
{ }

bool StaticModule::handleRequest(const HTTPRequest& req, HTTPResponse* resp) {
    std::string path = req.getPath();

    // TODO: sanitize path: filter out %20, make sure '..' won't go up a directory

    std::string filepath;
    if (filebase_[filebase_.size()-1] == '/') {
        filepath = filebase_ + req.getPath().substr(1);
    } else {
        filepath = filebase_ + req.getPath();
    }

    FileLoader fl;

    BinaryData data;
    FileLoaderError err = fl.openFile(filepath, &data);

    switch (err) {
        case FileErr_NoFile:
            resp->setError(HTTPResponseCode_404_NotFound);
            return true;
        case FileErr_NoPermission:
            resp->setError(HTTPResponseCode_401_Unauthorized);
            return true;
        case FileErr_IOErr:
            resp->setError(HTTPResponseCode_500_InternalServerError);
            return true;
        case FileErr_None:
            MIMEType mt = fl.guessMIMEType();
            std::string contentType = MIMETypeMap.at(mt);
            resp->okaySetContent(data, contentType);
            return true;
    }

    resp->okaySetContent(req.getRawRequest(), HTTPContentType_Plain);
    return false;
}
