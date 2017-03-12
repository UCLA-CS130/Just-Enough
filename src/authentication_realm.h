#pragma once

#include <string>
#include <map>

class AuthenticationRealm {
    public:
        AuthenticationRealm(std::string realm, std::map<std::string, std::string> users);

        bool authenticate(std::string username, std::string password);
        std::string realm();

    private:
        std::map<std::string, std::string> users_;
        std::string realm_;
};
