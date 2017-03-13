#include "authentication_realm.h"

AuthenticationRealm::AuthenticationRealm(std::string realm, std::map<std::string, std::string> users)
    : users_(users), realm_(realm)
{}

bool AuthenticationRealm::authenticate(std::string username, std::string password) {
    auto found = users_.find(username);
    if (found != users_.end()) {
        return (found->second == password);
    }
    return false;
}

std::string AuthenticationRealm::realm() {
    return realm_;
}
