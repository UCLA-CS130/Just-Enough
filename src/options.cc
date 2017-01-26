#include <string>

#include "options.h"

Options::Options(const char* file_name) {
	NginxConfigParser parser;
	parser.Parse(file_name, &config);
}

short Options::getPort() {
	for (unsigned int i =0; i < config.statements_.size();i++) {
		if (config.statements_[i]->tokens_[0] == "server") {
			std::shared_ptr<NginxConfigStatement> temp_config = config.statements_[i];
			for (unsigned int j = 0; j < temp_config->child_block_->statements_.size(); j++) {
				if(temp_config->child_block_->statements_[j]->tokens_[0] == "listen") {
					std::string port = temp_config->child_block_->statements_[j]->tokens_[1];
					return (short) std::stoi(port);
				}
			}
		}
	}
    return -1;
}