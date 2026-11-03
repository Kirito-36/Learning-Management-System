#pragma once
#include <string>
#include <vector>

#include "domain/AuditEntry.hpp"

namespace lms::data {

class IAuditRepository {
public:
    virtual ~IAuditRepository() = default;
    virtual void append(const domain::AuditEntry& entry) = 0;
    // Oldest first.
    virtual std::vector<domain::AuditEntry> findByEntity(const std::string& entityType,
                                                         int entityId) const = 0;
};

}  // namespace lms::data
