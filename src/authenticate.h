#pragma once
#include <string>
#include <vector>
#include "request_handler.h"
#include "authentication_realm.h"

class Authentication {
    public:
    virtual bool requestRequiresAuthentication(const Request& req);
    virtual bool requestPassesAuthentication(const Request& req);
    virtual void generateFailedAuthenticationResponse(const Request& req, Response* resp);

    void addRealm(AuthenticationRealm* r);

    virtual ~Authentication() = default;

    private:
    std::map<std::string, std::unique_ptr<AuthenticationRealm>> realms_;
};
