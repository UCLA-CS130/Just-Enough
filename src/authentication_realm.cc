#include "authentication_realm.h"

AuthenticationRealm::AuthenticationRealm(std::map<std::string, std::string> users)
    : users_(users)
{}

bool AuthenticationRealm::authenticate(std::string username, std::string password) {
    auto found = users_.find(username);
    if (found != users_.end()) {
        return (found->second == password);
    }
    return false;
}
