#include "authenticate.h"
#include "utils.h"
#include "base64.h"

#include <memory>

bool Authentication::requestRequiresAuthentication(const Request& req) {
    return (mapHasPrefix(realms_, req.uri()) != nullptr);
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
            break;
        }
    }
    if ( ! found_auth) {
        return false;
    }

    auto found = mapHasPrefix(realms_, req.uri());
    if (found) {
        return (*found)->authenticate(user, pass);
    }
    return false;
}

void Authentication::generateFailedAuthenticationResponse(const Request& req, Response* resp) {
    resp->SetStatus(Response::code_401_unauthorized);
    resp->AddHeader("Content-Type", "text/html");
    auto found = realms_.find(req.uri());
    if (found != realms_.end() && found->second) {
        resp->AddHeader("WWW-Authenticate", "Basic realm=\"" + found->second->realm() + "\"");
    }
    resp->SetBody("<html><body><h1>401 Unauthorized</h1><p>Invalid credentials</p></body></html>");
}

void Authentication::addRealm(AuthenticationRealm* r) {
    realms_[r->realm()] = std::unique_ptr<AuthenticationRealm>(r);
}


