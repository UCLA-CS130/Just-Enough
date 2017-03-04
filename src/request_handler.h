#pragma once
#include <string>
#include "config_parser.h"
#include "register_handler.h"


// For the Request and Response classes, you need to implement the methods
// and add private data as appropriate. You may also need to modify or extend
// the API when implementing the reverse proxy. Use your good judgment.

// Represents an HTTP Request.
//
// Usage:
//   auto request = Request::Parse(raw_request);
class Request {
    public:
        static std::unique_ptr<Request> Parse(const std::string& raw_request);

        /*
         * Mutators
         */

        void set_uri(const std::string &uri)
        {
          uri_ = uri;
        }

        void add_header(const std::string &lhs, const std::string &rhs)
        {
          headers_.push_back(std::pair<std::string,std::string>(lhs, rhs));
        }

        void remove_header(const std::string &lhs)
        {
          Headers::iterator it;
          for (it = headers_.begin() ; it != headers_.end(); ++it) {
            if ((*it).first == lhs) {
              headers_.erase(it);
              return;
            }
          }
        }

        /*
         * Accessors
         */

        std::string raw_request() const;
        std::string method() const;
        std::string uri() const;
        std::string version() const; // full version, ex: "HTTP/1.1"

        using Headers = std::vector<std::pair<std::string, std::string>>;
        Headers headers() const;

        std::string body() const;

    private:
        std::string uri_;
        std::string version_;
        std::string method_;
        Headers headers_;
        std::string body_;
};

// Represents an HTTP response.
//
// Usage:
//   Response r;
//   r.SetStatus(RESPONSE_200);
//   r.SetBody(...);
//   return r.ToString();
//
// Constructed by the RequestHandler, after which the server should call ToString
// to serialize.
class Response {
    public:
        enum ResponseCode {
            code_200_OK = 200,

            code_302_found = 302, // aka redirect

            code_400_bad_request = 400,
            code_401_unauthorized = 401,
            code_403_forbidden = 403,
            code_404_not_found = 404,

            code_500_internal_error = 500,
        };

        void SetStatus(const ResponseCode response_code);
        ResponseCode status() const;
        void AddHeader(const std::string& header_name, const std::string& header_value);
        void SetBody(const std::string& body);

        std::string ToString();

    private:
        ResponseCode code_;
        std::string body_;
        using Headers = std::vector<std::pair<std::string, std::string>>;
        Headers headers_;
};

// Represents the parent of all request handlers. Implementations should expect to
// be long lived and created at server constrution.
class RequestHandler {
    public:
        enum Status {
            OK = 0,
            Error,
        };

        // Initializes the handler. Returns a response code indicating success or
        // failure condition.
        // uri_prefix is the value in the config file that this handler will run for.
        // config is the contents of the child block for this handler ONLY.
        virtual Status Init(const std::string& uri_prefix,
                const NginxConfig& config) = 0;

        // Handles an HTTP request, and generates a response. Returns a response code
        // indicating success or failure condition. If ResponseCode is not OK, the
        // contents of the response object are undefined, and the server will return
        // HTTP code 500.
        virtual Status HandleRequest(const Request& request,
                Response* response) = 0;

        static RequestHandler* CreateByName(const char* type);

        virtual ~RequestHandler() = default;

        // get type of request handler
        virtual std::string type() const = 0;
};

#define REGISTER_REQUEST_HANDLER(ClassName) \
  static RequestHandlerRegisterer<ClassName> ClassName##__registerer(#ClassName)
