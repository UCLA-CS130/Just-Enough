#pragma once
#include <string>
#include <map>
#include "http_request.h"

/*
HTTP/1.1 200 OK
Date: Sun, 18 Oct 2012 10:36:20 GMT
Server: Apache/2.2.14 (Win32)
Content-Length: 230
Connection: Closed
Content-Type: text/html; charset=iso-8859-1
*/

enum HTTPResponseCode {
    HTTPResponseCode_200_OK,

    HTTPResponseCode_400_BadRequest,
    HTTPResponseCode_401_Unauthorized,
    HTTPResponseCode_403_Forbidden,
    HTTPResponseCode_404_NotFound,

    HTTPResponseCode_500_InternalServerError,
    HTTPResponseCode_501_NotImplemented,
    HTTPResponseCode_505_HTTPVersionNotSupported,
};

extern const std::map<HTTPResponseCode, std::string> httpResponseCodeStrings;
const std::string HTTPContentType_Plain = "text/plain";
const std::string HTTPContentType_HTML = "text/html";

class HTTPResponse {
    public:
        HTTPResponse() : code_(HTTPResponseCode_200_OK) {};

        // set as a 200 OK
        void okaySetContent(std::string content, std::string contentType) {
            code_ = HTTPResponseCode_200_OK;
            content_ = content;
            contentType_ = contentType;
        }

        void setErrorFromHTTPRequestError(HTTPRequestError err);

        // set as a non-200 response, and generate html error message
        void setError(HTTPResponseCode code);

        // set as a non-200 reponse, but give a custom content
        void setErrorWithContent(HTTPResponseCode code, std::string content, std::string contentType);

        std::string makeResponseString();

    protected:
        HTTPResponseCode code_;

        std::string content_;
        std::string contentType_;
};
