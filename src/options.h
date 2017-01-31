#include "config_parser.h"

struct Options {
	unsigned short port;
	void loadOptionsFromFile(const char* filename);

	public:
		unsigned short getPort();
};

