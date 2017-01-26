#include <string>

#include "options.h"

short Options::getPort(const char* file_name) {
	NginxConfigParser parser;
    NginxConfig config;

    parser.Parse(file_name, &config);

    std::string p = config.statements_[0]->child_block_->statements_[0]->tokens_[1];
    return (short) std::stoi(p);
}