#include "echo_handler.h"
#include <iostream>

RequestHandler::Status EchoHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    (void) uri_prefix;
    (void) config;
    return RequestHandler::OK;
}

RequestHandler::Status EchoHandler::HandleRequest(const Request& req, Response* resp) {
    resp->SetStatus(Response::code_200_OK);
    resp->SetBody(req.raw_request());
    resp->AddHeader("Content-Type", "text/plain");
    return RequestHandler::OK;
}
