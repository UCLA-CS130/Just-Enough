#include "status_handler.h"
#include <iostream>

RequestHandler::Status StatusHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    return RequestHandler::OK;
}

RequestHandler::Status StatusHandler::HandleRequest(const Request& req, Response* resp) {
    resp->SetStatus(Response::code_200_OK);
    resp->SetBody("<html><body>Webserver Status<body></html>");
    resp->AddHeader("Content-Type", "text/html");
    return RequestHandler::OK;
}
