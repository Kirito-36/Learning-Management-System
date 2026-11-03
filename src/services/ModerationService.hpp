#pragma once
#include <string>
#include <vector>

#include "data/IAssignmentRepository.hpp"
#include "data/ICourseRepository.hpp"
#include "data/IMarkRepository.hpp"
#include "data/ISubmissionRepository.hpp"
#include "data/IUnitOfWork.hpp"
#include "domain/IClock.hpp"
#include "services/AuditService.hpp"

namespace lms::services {

// Head of Department use cases: review every mark in the department, then release it.
class ModerationService {
public:
    ModerationService(data::ICourseRepository& courses, data::IAssignmentRepository& assignments,
                      data::ISubmissionRepository& submissions, data::IMarkRepository& marks, AuditService& audit,
                      data::IUnitOfWork& uow, const domain::IClock& clock);

    std::vector<domain::Mark> pendingMarks(const domain::User& hod) const;
    std::vector<domain::Mark> approvedMarks(const domain::User& hod) const;

    // Approve, or request changes (a comment is then mandatory). Records a ModerationRecord.
    void review(const domain::User& hod, int markId, domain::ModerationDecision decision, const std::string& comment);

    // Makes an approved mark visible to the student.
    void release(const domain::User& hod, int markId);

private:
    domain::Mark loadForDepartment(const domain::User& hod, int markId, domain::Submission& submissionOut) const;

    data::ICourseRepository& courses_;
    data::IAssignmentRepository& assignments_;
    data::ISubmissionRepository& submissions_;
    data::IMarkRepository& marks_;
    AuditService& audit_;
    data::IUnitOfWork& uow_;
    const domain::IClock& clock_;
};

}  // namespace lms::services
