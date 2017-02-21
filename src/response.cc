#include "request_handler.h"

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using std::string;
using std::vector;

void Response::SetStatus(const Response::ResponseCode response_code) {
    code_ = response_code;
}
void Response::AddHeader(const string& header_name, const string& header_value) {
    headers_.push_back(std::pair<string, string>(header_name, header_value));
}
void Response::SetBody(const string& body) {
    body_ = body;
}

const static std::map<Response::ResponseCode, std::string> responseCodeStrings =
{
    {Response::code_200_OK, "200 OK"},

    {Response::code_400_bad_request, "400 Bad Request"},
    {Response::code_401_unauthorized, "401 Unauthorized"},
    {Response::code_403_forbidden, "403 Forbidden"},
    {Response::code_404_not_found, "404 Not Found"},

    {Response::code_500_internal_error, "500 Internal Server Error"},
};

string Response::ToString() {
    const std::string CRLF = "\r\n";
    std::string resp = "HTTP/1.1 " + responseCodeStrings.at(code_) + CRLF;

    for (auto& header : headers_) {
        resp += header.first + ": " + header.second + CRLF;
    }
    resp += CRLF;
    resp += body_;

    return resp;
}
