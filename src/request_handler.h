// For the Request and Response classes, you need to implement the methods
// and add private data as appropriate. You may also need to modify or extend
// the API when implementing the reverse proxy. Use your good judgment.

// Represents an HTTP Request.
//
// Usage:
//   auto request = Request::Parse(raw_request);
class Request {
    public:
        static unique_ptr<Request> Parse(const std::string& raw_request);

        std::string raw_request() const;
        std::string method() const;
        std::string uri() const;
        std::string version() const;

        using Headers = std::vector<std::pair<std::string, std::string>>;
        Headers headers() const;

        std::string body() const;
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
            // Define your HTTP response codes here.
            Response_200_OK = 200,
            Response_404_Not_Found = 404,
            Response_500_Internal_Error = 500,
        };

        void SetStatus(const ResponseCode response_code);
        void AddHeader(const std::string& header_name, const std::string& header_value);
        void SetBody(const std::string& body);

        std::string ToString();
};

// Represents the parent of all request handlers. Implementations should expect to
// be long lived and created at server constrution.
class RequestHandler {
    public:
        enum Status {
            OK = 0;
            // Define your status codes here.
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
};
