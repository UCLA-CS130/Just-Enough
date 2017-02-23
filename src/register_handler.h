#pragma once
#include <map>
#include <memory>
#include <string>

class RequestHandler; // forward declaration
extern std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders;

template<typename T>
class RequestHandlerRegisterer {
    public:
        RequestHandlerRegisterer(const std::string& type) {
            if (request_handler_builders == nullptr) {
                request_handler_builders = new std::map<std::string, RequestHandler* (*)(void)>;
            }
            (*request_handler_builders)[type] = RequestHandlerRegisterer::Create;
        }
        static RequestHandler* Create() {
            return new T;
        }
};
