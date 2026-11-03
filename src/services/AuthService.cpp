#include "services/AuthService.hpp"

#include "domain/Errors.hpp"

namespace lms::services {

AuthService::AuthService(const data::IUserRepository& users, const domain::IPasswordHasher& hasher)
    : users_(users), hasher_(hasher), decoyHash_(hasher.hash("decoy-password-not-used")) {}

std::unique_ptr<domain::User> AuthService::login(const std::string& username, const std::string& password) const {
    static const char* const kMessage = "Invalid username or password.";
    if (username.empty() || username.size() > 64 || password.empty() || password.size() > 256) {
        throw domain::AuthenticationError(kMessage);
    }
    auto user = users_.findByUsername(username);
    if (!user) {
        hasher_.verify(decoyHash_, password);
        throw domain::AuthenticationError(kMessage);
    }
    if (!user->authenticate(hasher_, password)) throw domain::AuthenticationError(kMessage);
    return user;
}

}  // namespace lms::services
