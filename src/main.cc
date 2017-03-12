#include <iostream>

#include "options.h"
#include "webserver.h"
#include "authenticate.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Invalid number of arguments. Usage: webserver <config_file>\n";
        return 1;
    }

    Options opt;
    if(opt.loadOptionsFromFile(argv[1]) == false) {
        std::cerr << "Did not parse config file.\n";
        return 1;
    }
    unsigned short port_num = opt.port;
    std::cout << "configured port: " << port_num << std::endl;

    std::cout << "registered handlers:" << std::endl;
    for (auto& hand : opt.handlerMap) {
        std::cout << "    [" << hand.first << "]: " << hand.second << std::endl;
    }

    Authentication* auth = new Authentication();
    auth->addRealm(new AuthenticationRealm("/echo/secret", {{"evan", "pass"}, {"admin", "password"}}));
    opt.auth = auth;

    Webserver ws(&opt);
    ws.run();

    return 0;
}
