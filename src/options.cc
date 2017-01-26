#include <string>

#include "options.h"

Options::Options(const char* file_name) {
	NginxConfigParser parser;
	parser.Parse(file_name, &config);
}

short Options::getPort() {
    std::string p = config.statements_[0]->child_block_->statements_[0]->tokens_[1];
    return (short) std::stoi(p);
}