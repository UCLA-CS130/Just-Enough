#include "static_handler.h"
#include "fileLoader.h"
#include "markdown_to_html.h"
#include <iostream>
#include <boost/filesystem.hpp>

using std::string;
size_t STATEMENT_SIZE = 2;

RequestHandler::Status StaticHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    path_ = uri_prefix;
    bool issetMarkdown = false;
    for (size_t i = 0; i < config.statements_.size(); i++) {
        if (config.statements_[i]->tokens_.size() == STATEMENT_SIZE) {
            if (config.statements_[i]->tokens_[0] == "root") {
                filebase_ = config.statements_[i]->tokens_[1];
            } else if ( ! issetMarkdown && 
                        config.statements_[i]->tokens_[0] == "convert_markdown") {
                if (config.statements_[i]->tokens_[1] == "true") {
                    convert_markdown_ = true;
                    issetMarkdown = true;
                } else {
                    convert_markdown_ = false;
                    issetMarkdown = true;
                }
            } else {
                std::cerr << "Invalid Static Handler parameter." << std::endl;
                return RequestHandler::Error;
            }
        } else {
            std::cerr << "Invalid Static Handler config" << std::endl;
            return RequestHandler::Error;
        }
    }

    notFoundHandler_ = RequestHandler::CreateByName("NotFoundHandler");
    if ( ! notFoundHandler_) {
        std::cerr << "Static handler could not create NotFoundHandler." << std::endl;
        return RequestHandler::Error;
    }
    return RequestHandler::OK;
}

RequestHandler::Status StaticHandler::HandleRequest(const Request& req, Response* resp) {

    // TODO: sanitize path: filter out %20, make sure '..' won't go up a directory

    string reqPath = req.uri().substr(path_.size());
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
            return notFoundHandler_->HandleRequest(req, resp);
        case FileErr_NoPermission:
            resp->SetStatus(Response::code_401_unauthorized);
            return RequestHandler::OK;
        case FileErr_IOErr:
            resp->SetStatus(Response::code_500_internal_error);
            return RequestHandler::OK;
        case FileErr_None:
            MIMEType mt = fl.guessMIMEType();
            string contentType = MIMETypeMap.at(mt);

            resp->SetStatus(Response::code_200_OK);

            if (convert_markdown_) {
                string converted_md = convertMarkdownToHtml(data);
                if (converted_md == "") {
                    // Something went wrong in converting the markdown.
                    resp->SetStatus(Response::code_500_internal_error);
                    return RequestHandler::OK;
                }
                resp->AddHeader("Content-Type", MIMETypeMap.at(MIMEType_html));
                resp->SetBody(converted_md);
                return RequestHandler::OK;
            } else {
                resp->AddHeader("Content-Type", contentType);
                resp->SetBody(data);
                return RequestHandler::OK;
            }
    }

    resp->SetStatus(Response::code_500_internal_error);
    return RequestHandler::Error;
}


StaticHandler::~StaticHandler() {
    if (notFoundHandler_) {
        delete notFoundHandler_;
    }
}
