#pragma once
#include <string>
#include <vector>

#include "data/IAssignmentRepository.hpp"
#include "data/ICourseRepository.hpp"
#include "data/IMarkRepository.hpp"
#include "data/ISubmissionRepository.hpp"
#include "data/IUnitOfWork.hpp"
#include "domain/IClock.hpp"
#include "domain/ILatePenaltyStrategy.hpp"
#include "services/AuditService.hpp"
#include "services/DeadlinePolicy.hpp"

namespace lms::services {

class MarkingService {
public:
    MarkingService(data::ICourseRepository& courses, data::IAssignmentRepository& assignments,
                   data::ISubmissionRepository& submissions, data::IMarkRepository& marks,
                   const DeadlinePolicy& deadlines, const domain::ILatePenaltyStrategy& penalty, AuditService& audit,
                   data::IUnitOfWork& uow, const domain::IClock& clock);

    // Creates or revises a mark and sends it for moderation. Feedback is mandatory. Revising an
    // existing mark requires a reason; old and new values are written to the audit log.
    domain::Mark saveMark(const domain::User& lecturer, int submissionId, double rawMark,
                          const std::string& feedback, const std::string& reason);

    // Current (non-superseded) submissions for an assignment the lecturer teaches.
    std::vector<domain::Submission> submissionsToMark(const domain::User& lecturer, int assignmentId) const;

private:
    data::ICourseRepository& courses_;
    data::IAssignmentRepository& assignments_;
    data::ISubmissionRepository& submissions_;
    data::IMarkRepository& marks_;
    const DeadlinePolicy& deadlines_;
    const domain::ILatePenaltyStrategy& penalty_;
    AuditService& audit_;
    data::IUnitOfWork& uow_;
    const domain::IClock& clock_;
};

}  // namespace lms::services
