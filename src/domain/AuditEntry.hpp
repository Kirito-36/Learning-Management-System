#pragma once
#include <optional>
#include <string>

#include "domain/Types.hpp"

namespace lms::domain {

// Who, when, old value, new value, why. Rows are append-only (enforced in the schema).
struct AuditEntry {
    int id = 0;
    std::optional<int> actorUserId;
    std::string action;
    std::string entityType;
    int entityId = 0;
    std::optional<std::string> oldValue;
    std::optional<std::string> newValue;
    std::optional<std::string> reason;
    Timestamp occurredAt = 0;
};

}  // namespace lms::domain
