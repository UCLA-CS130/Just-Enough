#include "static_module.h"
#include "fileLoader.h"
#include <iostream>
#include <boost/filesystem.hpp>

using std::string;

const string StaticModule::typeString = "static";

Module* StaticModule::createFromParameters(std::string path, std::shared_ptr<std::map<string, string>> params) {
    auto filebaseParam = params->find("filebase");
    if (filebaseParam == params->end()) {
        std::cerr << "Static Module must specify 'filebase' parameter." << std::endl;
        return nullptr;
    }

    string filebase = filebaseParam->second;
    if ( ! boost::filesystem::exists(filebase)) {
        std::cerr << "Invalid 'filebase' parameter for Static Module: directory '" << filebase << "' not found." << std::endl;
        return nullptr;
    }

    Module* mod = new StaticModule(path, filebase);
    return mod;
}

StaticModule::StaticModule(string path, string filebase)
    : Module(path), filebase_(filebase)
{ }

bool StaticModule::handleRequest(const HTTPRequest& req, HTTPResponse* resp) {
    std::cout << "  > request being handled by static module (path: " << path_ << ")" << std::endl;
    string reqPath = req.getPath().substr(path_.size());

    // TODO: sanitize path: filter out %20, make sure '..' won't go up a directory

    string filepath;
    if (filebase_[filebase_.size()-1] == '/') {
        filepath = filebase_ + reqPath.substr(1);
    } else {
        filepath = filebase_ + reqPath;
    }

    std::cout << "  > looking for '" << req.getPath() << "' (" << reqPath << ")" << " in " << filepath << std::endl;

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
            string contentType = MIMETypeMap.at(mt);
            resp->okaySetContent(data, contentType);
            return true;
    }

    resp->setError(HTTPResponseCode_500_InternalServerError);
    return false;
}
