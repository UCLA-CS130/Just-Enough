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

bool StaticModule::handleRequest(const Request& req, Response* resp) {
    std::cout << "  > request being handled by static module (path: " << path_ << ")" << std::endl;
    string reqPath = req.uri().substr(path_.size());

    // TODO: sanitize path: filter out %20, make sure '..' won't go up a directory

    string filepath;
    if (filebase_[filebase_.size()-1] == '/') {
        filepath = filebase_ + reqPath.substr(1);
    } else {
        filepath = filebase_ + reqPath;
    }

    if (boost::filesystem::is_directory(filepath)) {
        if (filepath[filepath.size()-1] != '/') {
            filepath += '/';
        }
        filepath += "index.html";
    }

    std::cout << "  > looking for '" << req.uri() << "' (" << reqPath << ")" << " in " << filepath << std::endl;

    FileLoader fl;

    BinaryData data;
    FileLoaderError err = fl.openFile(filepath, &data);

    switch (err) {
        case FileErr_NoFile:
            resp->SetStatus(Response::code_404_not_found);
            return true;
        case FileErr_NoPermission:
            resp->SetStatus(Response::code_401_unauthorized);
            return true;
        case FileErr_IOErr:
            resp->SetStatus(Response::code_500_internal_error);
            return true;
        case FileErr_None:
            MIMEType mt = fl.guessMIMEType();
            string contentType = MIMETypeMap.at(mt);

            resp->SetStatus(Response::code_200_OK);
            resp->AddHeader("Content-Type", contentType);
            resp->SetBody(data);
            return true;
    }

    resp->SetStatus(Response::code_500_internal_error);
    return false;
}
