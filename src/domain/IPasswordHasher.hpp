#pragma once
// Port for password hashing. The domain depends on this interface only; the
// libsodium implementation lives in src/security (Dependency Inversion).
#include <string>

namespace lms::domain {

class IPasswordHasher {
public:
    virtual ~IPasswordHasher() = default;
    // Returns a self-describing salted hash string. Throws std::runtime_error on failure.
    virtual std::string hash(const std::string& password) const = 0;
    virtual bool verify(const std::string& storedHash, const std::string& password) const = 0;
};

}  // namespace lms::domain
