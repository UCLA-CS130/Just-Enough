#include "config_parser.h"

struct Options {
	unsigned short port;
	bool loadOptionsFromFile(const char* filename);
};

