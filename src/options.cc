#include <string>

#include "options.h"

bool Options::loadOptionsFromFile(const char* filename) {
	//TODO: Does parse return error for bad filename
	NginxConfigParser parser;
	NginxConfig config;
	if(parser.Parse(filename, &config) == false) {
		return false;
	}

	bool issetPort = false;

	for (unsigned int i =0; i < config.statements_.size();i++) {

		// Sets the port
		if (!issetPort && config.statements_[i]->tokens_[0] == "server") {
			std::shared_ptr<NginxConfigStatement> temp_config = config.statements_[i];
			for (unsigned int j = 0; j < temp_config->child_block_->statements_.size(); j++) {
				if(temp_config->child_block_->statements_[j]->tokens_[0] == "listen") {
					std::string port = temp_config->child_block_->statements_[j]->tokens_[1];
					if((unsigned int) std::stoi(port) > 65535 || (unsigned int) std::stoi(port) < 1024) {
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