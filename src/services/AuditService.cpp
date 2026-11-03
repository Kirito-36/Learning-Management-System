#include "services/AuditService.hpp"

#include <utility>

#include "services/Guards.hpp"

namespace lms::services {

void AuditService::record(std::optional<int> actorUserId, const std::string& action,
                          const std::string& entityType, int entityId, std::optional<std::string> oldValue,
                          std::optional<std::string> newValue, std::optional<std::string> reason) {
    domain::AuditEntry e;
    e.actorUserId = actorUserId;
    e.action = action;
    e.entityType = entityType;
    e.entityId = entityId;
    e.oldValue = std::move(oldValue);
    e.newValue = std::move(newValue);
    e.reason = std::move(reason);
    e.occurredAt = clock_.now();
    repo_.append(e);
}

std::vector<domain::AuditEntry> AuditService::history(const domain::User& requester,
                                                      const std::string& entityType, int entityId) const {
    requirePermission(requester, domain::Permission::ViewAuditLog, "view the audit log");
    return repo_.findByEntity(entityType, entityId);
}

}  // namespace lms::services
