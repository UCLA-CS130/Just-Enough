#include "status_handler.h"
#include "webserver.h"
#include <iostream>

RequestHandler::Status StatusHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    return RequestHandler::OK;
}

RequestHandler::Status StatusHandler::HandleRequest(const Request& req, Response* resp) {
    Webserver* ws = Webserver::instance;
    if ( ! ws) {
        std::cerr << "StatusHandler failed: No global webserver instance." << std::endl;
        resp->SetStatus(Response::code_404_not_found);
        return RequestHandler::Error;
    }

    std::string html = (
            "<html>"
            "  <body>"
            "    Webserver Status"
            "  <body>"
            "</html>"
            );

    resp->SetStatus(Response::code_200_OK);
    resp->SetBody(html);
    resp->AddHeader("Content-Type", "text/html");
    return RequestHandler::OK;
}
