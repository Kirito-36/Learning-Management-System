#pragma once
#include <optional>
#include <string>
#include <vector>

#include "data/IAuditRepository.hpp"
#include "domain/IClock.hpp"
#include "domain/User.hpp"

namespace lms::services {

// Records who did what, when, to which entity, with old and new values and a reason.
class AuditService {
public:
    AuditService(data::IAuditRepository& repo, const domain::IClock& clock) : repo_(repo), clock_(clock) {}

    void record(std::optional<int> actorUserId, const std::string& action, const std::string& entityType,
                int entityId, std::optional<std::string> oldValue = std::nullopt,
                std::optional<std::string> newValue = std::nullopt,
                std::optional<std::string> reason = std::nullopt);

    // Administrators and Heads of Department only.
    std::vector<domain::AuditEntry> history(const domain::User& requester, const std::string& entityType,
                                            int entityId) const;

private:
    data::IAuditRepository& repo_;
    const domain::IClock& clock_;
};

}  // namespace lms::services
