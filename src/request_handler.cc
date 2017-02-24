#include "request_handler.h"
#include "echo_handler.h"
#include <cstring>

RequestHandler* RequestHandler::CreateByName(const char* type) {
    if ( ! request_handler_builders) {
        std::cerr << "No Request Handlers registered!" << std::endl;
    }
    const auto type_and_builder = request_handler_builders->find(type);
    if (type_and_builder == request_handler_builders->end()) {
        std::cerr << "handler type '" << type << "' not registered." << std::endl;
        return nullptr;
    }
    return (*type_and_builder->second)();
}
