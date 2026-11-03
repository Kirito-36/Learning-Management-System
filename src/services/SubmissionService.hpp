#pragma once
#include <string>
#include <vector>

#include "data/IAssignmentRepository.hpp"
#include "data/ICourseRepository.hpp"
#include "data/ISubmissionRepository.hpp"
#include "data/IUnitOfWork.hpp"
#include "domain/IClock.hpp"
#include "domain/ILatePenaltyStrategy.hpp"
#include "services/AuditService.hpp"
#include "services/DeadlinePolicy.hpp"

namespace lms::services {

struct SubmitResult {
    domain::Submission submission;
    domain::LateResult lateness;
};

class SubmissionService {
public:
    SubmissionService(data::ICourseRepository& courses, data::IAssignmentRepository& assignments,
                      data::ISubmissionRepository& submissions, const DeadlinePolicy& deadlines,
                      const domain::ILatePenaltyStrategy& penalty, AuditService& audit, data::IUnitOfWork& uow,
                      const domain::IClock& clock);

    // First attempts are accepted at any time (late work is flagged, never rejected); the penalty
    // is applied at marking. Resubmission is allowed only before the effective deadline, while the
    // previous attempt has not been marked, and within the assignment's attempt limit.
    SubmitResult submit(const domain::User& student, int assignmentId, const std::string& filePath);

    std::vector<domain::Submission> mySubmissions(const domain::User& student) const;

private:
    data::ICourseRepository& courses_;
    data::IAssignmentRepository& assignments_;
    data::ISubmissionRepository& submissions_;
    const DeadlinePolicy& deadlines_;
    const domain::ILatePenaltyStrategy& penalty_;
    AuditService& audit_;
    data::IUnitOfWork& uow_;
    const domain::IClock& clock_;
};

}  // namespace lms::services
