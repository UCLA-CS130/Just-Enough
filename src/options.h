#include "config_parser.h"

class Options {
	public:
		Options(const char* file_name);
		unsigned short getPort();
	private:
		NginxConfig config;
};
