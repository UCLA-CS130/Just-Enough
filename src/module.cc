#include <string>
#include <map>
#include <iostream>

#include "module.h"

//const std::string firstEntry[3] = {"type", "path", "filebase"};

//Easier way to program this?
Module::Module() {
	std::map<std::string, std::string> moduleParameters;
}

bool Module::add(const std::string& str, const std::string& str2) {
	if(str.empty() || str2.empty()) {
		std::cerr << "Entries to module are empty.\n";
		return false;
	}

	//TODO:: A refactor into an array check?
	if(!(str == "type" || str == "path")) {
		std::cerr << "First entry to module is not an acceptable input.\n";
		return false;
	}

	if(moduleParameters.find(str) == moduleParameters.end()) {
		moduleParameters[str] = str2;
		return true;
	}

	std::cerr << "Failed to add to module map.\n";
	return false;
}

const std::string Module::getPathName(const std::string& str) {
	std::map<std::string, std::string>::iterator it;
	it = moduleParameters.find(str);
	if(it == moduleParameters.end()) {
		std::cerr << "Could not find particular path in module map.\n";
		return "";
	}
	return it->second;
}