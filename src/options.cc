#include <string>

#include "options.h"

void Options::loadOptionsFromFile(const char* filename) {
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
					this->port = (unsigned short) std::stoi(port);
				}
			}
		}
	}
}

unsigned short Options::getPort() {
	//TODO check port here
	return this->port;
};
