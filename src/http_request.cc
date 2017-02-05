#include "http_request.h"
#include "utils.h"
#include <sstream>
#include <iostream>
#include <vector>

using std::string;
using std::vector;

HTTPRequestError HTTPRequest::loadFromRawRequest(const string& reqStr) {
    std::stringstream reqSS(reqStr);

    string head = getCRLFLine(reqSS);
    vector<string> headTokens = split(head, ' ');
    if (headTokens.size() != 3) { // VERB PATH HTTP/VERSION
        return HTTPRequestError_Malformed;
    }
    if (headTokens[0] == "GET") {
        verb_ = HTTPVerb_GET;
    } else if (headTokens[0] == "POST") {
        verb_ = HTTPVerb_POST;
    } else {
        return HTTPRequestError_BadVerb;
    }

    path_ = headTokens[1];

    // HTTP/1.1
    if (headTokens[2].substr(0, 5) != "HTTP/") {
        return HTTPRequestError_Malformed;
    }
    if (headTokens[2].substr(5) != "1.1") {
        // TODO: be ok with 1.0?
        return HTTPRequestError_BadVersion;
    }

    while (true) {
        string line = getCRLFLine(reqSS);
        if (line == "") break;

        size_t delimPos = line.find(':');
        if (delimPos == std::string::npos) {
            return HTTPRequestError_Malformed;
        }

        std::stringstream lineSS(line);
        string headerKey = line.substr(0, delimPos);
        size_t headerValueStart = delimPos+1; // after ':'
        while (headerValueStart < line.size() && line[headerValueStart] == ' ') {
            headerValueStart++;
        }
        string headerValue = line.substr(headerValueStart);

        headers_[headerKey] = headerValue;
    }


    return HTTPRequestError_None;
}

