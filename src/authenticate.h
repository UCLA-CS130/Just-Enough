#pragma once
#include <string>
#include <vector>
#include "request_handler.h"
#include "authentication_realm.h"

class Authentication {
    public:
    bool requestRequiresAuthentication(const Request& req);
    bool requestPassesAuthentication(const Request& req);
    void generateFailedAuthenticationResponse(const Request& req, Response* resp);

    void addRealm(AuthenticationRealm* r);

    private:
    std::map<std::string, std::unique_ptr<AuthenticationRealm>> realms_;
};
