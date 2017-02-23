#include "request_handler.h"
#include "echo_handler.h"
#include <cstring>

RequestHandler* RequestHandler::CreateByName(const char* type) {
    // TODO(evan): use static registeration
    std::string typeStr(type, std::strlen(type));
    if (typeStr == "echo") {
        return new EchoHandler();
    } else {
        std::cerr << "Unknown handler name '" << typeStr << "'" << std::endl;
        return nullptr;
    }
}
