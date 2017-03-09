#include "request_handler.h"

#include "utils.h"
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

using std::string;
using std::vector;
using Headers = vector<std::pair<string, string>>;

std::unique_ptr<Request> Request::Parse(const string& raw_request) {
    std::stringstream reqSS(raw_request);

    std::unique_ptr<Request> req(new Request());

    string head = getCRLFLine(reqSS);
    vector<string> headTokens = split(head, ' ');
    if (headTokens.size() != 3) { // VERB PATH HTTP/VERSION
        std::cerr << "received malformed request: " << head << std::endl;
        return nullptr;
    }
    req->method_ = headTokens[0];
    // TODO: validate method_

    req->uri_ = headTokens[1];

    // HTTP/1.1
    if (headTokens[2].substr(0, 5) != "HTTP/") {
        std::cerr << "received malformed request: " << head << std::endl;
        return nullptr;
    }
    req->version_ = headTokens[2];

    int contentLength = 0;
    while (true) {
        string line = getCRLFLine(reqSS);
        if (line == "") break;

        size_t delimPos = line.find(':');
        if (delimPos == string::npos) {
            std::cerr << "received malformed request header: " << line << std::endl;
            return nullptr;
        }

        std::stringstream lineSS(line);
        string headerKey = line.substr(0, delimPos);
        size_t headerValueStart = delimPos+1; // after ':'
        while (headerValueStart < line.size() && line[headerValueStart] == ' ') {
            headerValueStart++;
        }
        string headerValue = line.substr(headerValueStart);

        if (headerKey == "Content-Length") {
            contentLength = std::strtol(headerValue.c_str(), nullptr, 10); // 0 on error
        }

        // TODO: check if header is given twice?
        req->headers_.push_back(std::pair<string,string>(headerKey, headerValue));
    }

    if (contentLength > 0) {
        // read contentLength characters directly into req->body
        req->body_.resize(contentLength, '\0');
        reqSS.read(&req->body_[0], contentLength);
        // request body is only filled if "Content-Type" is specified,
        // as per https://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html
    }

    return req;
}

string Request::raw_request() const {
    std::stringstream raw_request;
    raw_request << method() << " " << uri() << " " << version() << "\r\n";
    Headers hdrs = headers();
    for (auto const& hdr : hdrs) {
        raw_request << hdr.first << ": " << hdr.second << "\r\n";
    }
    raw_request << "\r\n";
    raw_request << body();
    return raw_request.str();
}
string Request::method() const {
    return method_;
}
string Request::uri() const {
    return uri_;
}
string Request::version() const {
    return version_;
}
Headers Request::headers() const {
    return headers_;
}
string Request::body() const {
    return body_;
}
