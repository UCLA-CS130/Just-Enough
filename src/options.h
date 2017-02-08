#pragma once
#include <vector>

#include "module.h"
#include "config_parser.h"

struct Options {
	unsigned short port;
	std::vector<Module*> modules;
	bool addModuleToVec(Module* module);
	bool addModule(std::shared_ptr<NginxConfigStatement> module_config);
	bool addPort(std::shared_ptr<NginxConfigStatement> port_config);
	bool loadOptionsFromStream(std::istream* config_file);
	bool loadOptionsFromFile(const char* filename);
	~Options();
};

