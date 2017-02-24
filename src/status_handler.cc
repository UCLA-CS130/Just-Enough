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

    auto opt = ws->options();
    std::string handlersStr;
    for (auto h : opt->handlerMap) {
        handlersStr += "<tr><td>" + h.first + "</td><td>" + h.second->type() + "</td></tr>";
    }

    std::string requestCountsStr;
    auto counters = ws->counters();
    for (auto k : counters->getKeys()) {
        std::cout << " > " << k << std::endl;
        requestCountsStr += "<ul><h3>" + k + "</h3>";
        for (auto c : counters->iterateKey(k)) {
            std::cout << "    - " << c.first << " = " << c.second << std::endl;
            requestCountsStr += "<li>" + std::to_string((int)c.first) + " : " + std::to_string(c.second) + "</li>";
        }
        requestCountsStr += "</ul>";
    }

    std::string html = (
            "<html>"
            "  <body>"
            "    <h1>Webserver Status</h1>"
            "    <h2>Registered handlers:</h2>"
            "    <table style=\"font-family: monospace\">"
            "      <tr><th>Path</th><th>Handler Type</th></tr>"
            + handlersStr +
            "    </table>"
            + requestCountsStr +
            "  <body>"
            "</html>"
            );

    resp->SetStatus(Response::code_200_OK);
    resp->SetBody(html);
    resp->AddHeader("Content-Type", "text/html");
    return RequestHandler::OK;
}
