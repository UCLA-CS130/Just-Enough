#include "config_parser.h"

class Options {
	public:
		Options(const char* file_name);
		short getPort();
	private:
		NginxConfig config;
};