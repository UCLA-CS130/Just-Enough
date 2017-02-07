#include <string>
#include <map>


class Module {
	public:
		Module();
		bool add(const std::string& str, const std::string& str2);
		const std::string getPathName(const std::string& str);

	private:
		std::map<std::string, std::string> moduleParameters;
};