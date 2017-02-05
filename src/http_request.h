#include <string>
#include <unordered_map>

/*
GET / HTTP/1.1
Host: localhost:8080
Connection: keep-alive
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.95 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,* / *;q=0.8
Accept-Encoding: gzip, deflate, sdch, br
Accept-Language: en-US,en;q=0.8
*/

enum HTTPVerb {
    HTTPVerb_GET = 0,
    HTTPVerb_POST,
    // TODO: add all verbs & implement them
};

enum HTTPRequestError {
    HTTPRequestError_None = 0,
    HTTPRequestError_Malformed,
    HTTPRequestError_BadVerb,
    HTTPRequestError_BadVersion,

    HTTPRequestError_404_NotFound,
    HTTPRequestError_500_InternalError,
};

class HTTPRequest {
    public:
        // if this returns any error but HTTPRequestError_None,
        // you should not use *any* other method
        HTTPRequestError loadFromRawRequest(const std::string& reqStr);

        HTTPVerb getVerb() {
            return verb_;
        }

        std::string getPath() {
            return path_;
        }

        std::string getHeader(const std::string& key) {
            auto found = headers_.find(key);
            if (found == headers_.end()) {
                return "";
            }
            return found->second;
        }

    protected:
        HTTPVerb verb_;
        std::string path_; // TODO: split path into path + query + fragment
        std::unordered_map<std::string, std::string> headers_;
};
