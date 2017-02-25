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
    if (opt->defaultHandler) {
        handlersStr += "<tr><td>(default)</td><td>" + opt->defaultHandler->type() + "</td></tr>";
    }

    std::string requestCountsStr;
    auto counters = ws->counters();
    for (auto k : counters->getKeys()) {
        bool first = true;
        for (auto c : counters->iterateKey(k)) {
            requestCountsStr += first ? "<tr class=\"overline\">" : "<tr>";
            requestCountsStr += "<td>" + k + "</td>";
            requestCountsStr += "<td>" + std::to_string((int)c.first) + "</td>";
            requestCountsStr += "<td>" + std::to_string(c.second) + "</td>";
            requestCountsStr += "</tr>";
            first = false;
        }
    }

    std::string html = (
            "<html>"
            "  <head>"
            "  <style>"
            "  table {"
            "    font-family: monospace;"
            "    border: 1px solid #777;"
            "    border-collapse: collapse"
            "  }"
            "  td {"
            "    padding: 4px 20px;"
            "  }"
            "  .underline {"
            "    border-bottom: 1px solid #777;"
            "  }"
            "  .overline {"
            "    border-top: 1px solid #777;"
            "  }"
            "  </style>"
            "  </head>"
            "  <body>"
            "    <h1>Webserver Status</h1>"
            "    <h2>Registered handlers:</h2>"
            "    <table>"
            "      <tr class=\"underline\"><th>Path</th><th>Handler Type</th></tr>"
            + handlersStr +
            "    </table>"
            "    <h2>Requests:</h2>"
            "    <table>"
            "      <tr class=\"underline\"><th>Path</th><th>Response Code</th><th>Count</th></tr>"
            + requestCountsStr +
            "    </table>"
            "  <body>"
            "</html>"
            );

    resp->SetStatus(Response::code_200_OK);
    resp->SetBody(html);
    resp->AddHeader("Content-Type", "text/html");
    return RequestHandler::OK;
}
