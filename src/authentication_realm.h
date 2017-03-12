#pragma once

#include <string>
#include <map>

class AuthenticationRealm {
    public:
        AuthenticationRealm(std::map<std::string, std::string> users);

        bool authenticate(std::string username, std::string password);

    private:
        std::map<std::string, std::string> users_;
};
