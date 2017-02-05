#include "http_response.h"

const std::map<HTTPResponseCode, std::string> httpResponseCodeStrings =
{
    {HTTPResponseCode_200_OK, "200 OK"},

    {HTTPResponseCode_400_BadRequest, "400 Bad Request"},
    {HTTPResponseCode_401_Unauthorized, "401 Unauthorized"},
    {HTTPResponseCode_403_Forbidden, "403 Forbidden"},
    {HTTPResponseCode_404_NotFound, "404 Not Found"},

    {HTTPResponseCode_500_InternalServerError, "500 Internal Server Error"},
    {HTTPResponseCode_501_NotImplemented, "501 Not Implemented"},
    {HTTPResponseCode_505_HTTPVersionNotSupported, "505 HTTP Version Not Supported"},
};

std::string HTTPResponse::makeResponseString() {
    const std::string CRLF = "\r\n";
    std::string resp = "HTTP/1.1 " + httpResponseCodeStrings.at(code_) + CRLF;

    resp += "Content-Type: " + contentType_ + CRLF;
    resp += "Connection: Closed" + CRLF;
    resp += CRLF;
    resp += content_;

    return resp;
}

void HTTPResponse::setErrorFromHTTPRequestError(HTTPRequestError err) {
    switch(err) {
        case HTTPRequestError_None:
            return;
        case HTTPRequestError_Malformed:
            return setError(HTTPResponseCode_400_BadRequest);
        case HTTPRequestError_BadVerb:
            return setError(HTTPResponseCode_400_BadRequest);
        case HTTPRequestError_BadVersion:
            return setError(HTTPResponseCode_505_HTTPVersionNotSupported);
    }
}

void HTTPResponse::setError(HTTPResponseCode code) {
    code_ = code;
    content_ = "<html><body><h1>" + httpResponseCodeStrings.at(code) + "</body></html>";
    contentType_ = "text/html";
}

void HTTPResponse::setErrorWithContent(HTTPResponseCode code, std::string content, std::string contentType) {
    code_ = code;
    content_ = content;
    contentType_ = contentType;
}
