#include "config_parser.h"

struct Options {
	unsigned short port;
	bool loadOptionsFromStream(std::istream* config_file);
	bool loadOptionsFromFile(const char* filename);
};

