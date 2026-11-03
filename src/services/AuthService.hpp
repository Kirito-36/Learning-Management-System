#pragma once
#include <memory>
#include <string>

#include "data/IUserRepository.hpp"
#include "domain/IPasswordHasher.hpp"

namespace lms::services {

class AuthService {
public:
    AuthService(const data::IUserRepository& users, const domain::IPasswordHasher& hasher);

    // Returns the authenticated user. Throws AuthenticationError with one generic message
    // for unknown user, wrong password or deactivated account (no user enumeration).
    std::unique_ptr<domain::User> login(const std::string& username, const std::string& password) const;

private:
    const data::IUserRepository& users_;
    const domain::IPasswordHasher& hasher_;
    std::string decoyHash_;  // verified for unknown usernames so timing does not reveal them
};

}  // namespace lms::services
