#include <string>

#include "options.h"

bool Options::loadOptionsFromFile(const char* filename) {
	NginxConfigParser parser;
	NginxConfig config;
	parser.Parse(filename, &config);

	for (unsigned int i =0; i < config.statements_.size();i++) {

		// Sets the port
		if (config.statements_[i]->tokens_[0] == "server") {
			std::shared_ptr<NginxConfigStatement> temp_config = config.statements_[i];
			for (unsigned int j = 0; j < temp_config->child_block_->statements_.size(); j++) {
				if(temp_config->child_block_->statements_[j]->tokens_[0] == "listen") {
					std::string port = temp_config->child_block_->statements_[j]->tokens_[1];
					if((unsigned int) std::stoi(port) > 65535 || (unsigned int) std::stoi(port) < 1024) {
						std::cerr << "Invalid port number.\n";
        				return false;
					}
					//TODO: check if port has already been set
					this->port = (unsigned short) std::stoi(port);
				}
			}
		}
	}
	return true;
}