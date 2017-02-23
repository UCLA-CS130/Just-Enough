#include "register_handler.h"

std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders = nullptr;
