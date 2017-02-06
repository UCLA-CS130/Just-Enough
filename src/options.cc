#include <string>
#include <fstream>

#include "options.h"

const int MAX_PORT = 65535;
const int MIN_PORT = 1024;

//TODO: A better styling of name might be better here
//TODO: Might also need to understand what = is
const int COM = 0;
const int ANS = 1;

bool Options::loadOptionsFromStream(std::istream* config_file) {

	NginxConfigParser parser;
	NginxConfig config;
	if(parser.Parse(config_file, &config) == false) {
		return false;
	}

	bool issetPort = false;

	for (unsigned int i =0; i < config.statements_.size();i++) {

		// Sets the port
		if (!issetPort && config.statements_[i]->tokens_[COM] == "server") {
			std::shared_ptr<NginxConfigStatement> temp_config = config.statements_[i];
			for (unsigned int j = 0; j < temp_config->child_block_->statements_.size(); j++) {
				if(temp_config->child_block_->statements_[j]->tokens_[COM] == "listen") {
					std::string port = temp_config->child_block_->statements_[j]->tokens_[ANS];
					if((unsigned int) std::stoi(port) > MAX_PORT || (unsigned int) std::stoi(port) < MIN_PORT) {
						std::cerr << "Invalid port number.\n";
        				return false;
					}
					if(issetPort) {
						std::cerr << "Multiple ports in config file.\n";
						return false;
					}
					this->port = (unsigned short) std::stoi(port);
					issetPort = true;
				}
			}
		}
	}
	return true;
}

bool Options::loadOptionsFromFile(const char* filename) {
	//TODO: Does parse return error for bad filename
	std::ifstream config_file;
	config_file.open(filename);
  	if (!config_file.good()) {
    	std::cerr << "Failed to open config file.\n";
    	return false;
	}

	bool val = loadOptionsFromStream(dynamic_cast<std::istream*>(&config_file));
	config_file.close();
	return val;
}