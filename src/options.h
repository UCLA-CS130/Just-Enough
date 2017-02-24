#pragma once
#include <map>
#include <vector>

#include "module.h"
#include "request_handler.h"
#include "config_parser.h"

struct Options {
	unsigned short port;
	std::map<std::string, RequestHandler*> handlerMap;
	RequestHandler* defaultHandler;
	bool addHandler(std::string type, std::string path, const NginxConfig& handler_config);
	bool addPath(std::shared_ptr<NginxConfigStatement> handler_config);
	bool addDefaultHandler(std::shared_ptr<NginxConfigStatement> handler_config);
	bool addPort(std::shared_ptr<NginxConfigStatement> port_config);
	bool loadOptionsFromStream(std::istream* config_file);
	bool loadOptionsFromFile(const char* filename);
	~Options();
};

