#pragma once
// Small shared checks used by the application services.
#include <optional>
#include <string>

#include "domain/Errors.hpp"
#include "domain/User.hpp"

namespace lms::services {

inline void requirePermission(const domain::User& user, domain::Permission p, const char* action) {
    if (!user.active() || !user.can(p)) {
        throw domain::AccessDenied(std::string("You are not allowed to ") + action + ".");
    }
}

// Department of a lecturer or Head of Department; empty for other roles.
inline std::optional<int> departmentOf(const domain::User& user) {
    if (const auto* lecturer = dynamic_cast<const domain::Lecturer*>(&user)) return lecturer->departmentId();
    return std::nullopt;
}

inline void requireText(const std::string& value, const char* field, std::size_t maxLength) {
    if (value.empty() || value.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw domain::ValidationError(std::string(field) + " must not be empty.");
    }
    if (value.size() > maxLength) {
        throw domain::ValidationError(std::string(field) + " must be at most " + std::to_string(maxLength) +
                                      " characters.");
    }
}

}  // namespace lms::services
