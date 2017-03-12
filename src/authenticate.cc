#include "authenticate.h"
#include "utils.h"
#include "base64.h"

#include <memory>

bool Authentication::requestRequiresAuthentication(const Request& req) {
    return (realms_.find(req.uri()) != realms_.end());
}

bool Authentication::requestPassesAuthentication(const Request& req) {
    bool found_auth = false;
    std::string user;
    std::string pass;
    for (auto header : req.headers()) {
        if (header.first == "Authorization") {

            std::string b64_part;
            auto tokens = split(header.second, ' ');
            if (tokens.size() != 2 || tokens[0] != "Basic") {
                std::cout << "unrecognized auth format: " << (tokens.size() > 0 ? tokens[0] : "<none>") << std::endl;
                return false;
            }

            std::string authUserPass = base64_decode(tokens[1]);
            auto userpass = split(authUserPass, ':');
            if (userpass.size() != 2) {
                std::cout << "unrecognized auth format " << std::endl;
                return false;
            }
            user = userpass[0];
            pass = userpass[1];

            found_auth = true;
        }
    }
    if ( ! found_auth) {
        return false;
    }

    auto found = realms_.find(req.uri());
    if (found != realms_.end() && found->second) {
        return found->second->authenticate(user, pass);
    }
    return false;
}

void Authentication::generateFailedAuthenticationResponse(const Request&, Response* resp) {
    resp->SetStatus(Response::code_401_unauthorized);
    resp->AddHeader("Content-Type", "text/html");
    resp->AddHeader("WWW-Authenticate", "Basic realm=\"MyServerRealm\"");
    resp->SetBody("<html><body><h1>401 Unauthorized</h1><p>Invalid credentials</p></body></html>");
}

void Authentication::addRealm(std::string uri, AuthenticationRealm* r) {
    realms_[uri] = std::unique_ptr<AuthenticationRealm>(r);
}


