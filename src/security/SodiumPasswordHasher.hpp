#pragma once
#include "domain/IPasswordHasher.hpp"

namespace lms::security {

// Argon2id via libsodium's crypto_pwhash_str. The returned string embeds the algorithm
// parameters and a random salt, so no separate salt column is needed.
// INTERACTIVE limits (about 64 MiB, 2 passes) keep login fast on the school's older PCs.
class SodiumPasswordHasher final : public domain::IPasswordHasher {
public:
    SodiumPasswordHasher();  // initialises libsodium; throws std::runtime_error on failure
    std::string hash(const std::string& password) const override;
    bool verify(const std::string& storedHash, const std::string& password) const override;
};

}  // namespace lms::security
