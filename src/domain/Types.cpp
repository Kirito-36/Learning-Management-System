#include "domain/Types.hpp"

#include <array>
#include <stdexcept>
#include <utility>

namespace lms::domain {
namespace {

template <typename E, std::size_t N>
using Table = std::array<std::pair<E, const char*>, N>;

template <typename E, std::size_t N>
std::string nameOf(const Table<E, N>& table, E value) {
    for (const auto& [e, n] : table) {
        if (e == value) return n;
    }
    throw std::invalid_argument("unmapped enumeration value");
}

template <typename E, std::size_t N>
E parse(const Table<E, N>& table, const std::string& text, const char* what) {
    for (const auto& [e, n] : table) {
        if (text == n) return e;
    }
    throw std::invalid_argument(std::string("unknown ") + what + ": '" + text + "'");
}

const Table<Role, 4> kRoles{{{Role::Administrator, "ADMIN"},
                              {Role::Lecturer, "LECTURER"},
                              {Role::HeadOfDepartment, "HOD"},
                              {Role::Student, "STUDENT"}}};

const Table<SubmissionStatus, 8> kSubmissionStatuses{
    {{SubmissionStatus::Draft, "DRAFT"},
     {SubmissionStatus::Submitted, "SUBMITTED"},
     {SubmissionStatus::UnderReview, "UNDER_REVIEW"},
     {SubmissionStatus::MarkedPendingModeration, "MARKED_PENDING_MODERATION"},
     {SubmissionStatus::ChangesRequested, "CHANGES_REQUESTED"},
     {SubmissionStatus::Approved, "APPROVED"},
     {SubmissionStatus::Released, "RELEASED"},
     {SubmissionStatus::Superseded, "SUPERSEDED"}}};

const Table<MarkStatus, 4> kMarkStatuses{{{MarkStatus::PendingModeration, "PENDING_MODERATION"},
                                          {MarkStatus::ChangesRequested, "CHANGES_REQUESTED"},
                                          {MarkStatus::Approved, "APPROVED"},
                                          {MarkStatus::Released, "RELEASED"}}};

const Table<ExtensionStatus, 3> kExtensionStatuses{{{ExtensionStatus::Pending, "PENDING"},
                                                    {ExtensionStatus::Approved, "APPROVED"},
                                                    {ExtensionStatus::Rejected, "REJECTED"}}};

const Table<ModerationDecision, 2> kDecisions{
    {{ModerationDecision::Approved, "APPROVED"},
     {ModerationDecision::ChangesRequested, "CHANGES_REQUESTED"}}};

const Table<EnrolmentStatus, 2> kEnrolmentStatuses{{{EnrolmentStatus::Active, "ACTIVE"},
                                                    {EnrolmentStatus::Withdrawn, "WITHDRAWN"}}};

}  // namespace

std::string toString(Role r) { return nameOf(kRoles, r); }
std::string toString(SubmissionStatus s) { return nameOf(kSubmissionStatuses, s); }
std::string toString(MarkStatus s) { return nameOf(kMarkStatuses, s); }
std::string toString(ExtensionStatus s) { return nameOf(kExtensionStatuses, s); }
std::string toString(ModerationDecision d) { return nameOf(kDecisions, d); }
std::string toString(EnrolmentStatus s) { return nameOf(kEnrolmentStatuses, s); }

Role roleFromString(const std::string& s) { return parse(kRoles, s, "role"); }
SubmissionStatus submissionStatusFromString(const std::string& s) {
    return parse(kSubmissionStatuses, s, "submission status");
}
MarkStatus markStatusFromString(const std::string& s) { return parse(kMarkStatuses, s, "mark status"); }
ExtensionStatus extensionStatusFromString(const std::string& s) {
    return parse(kExtensionStatuses, s, "extension status");
}
ModerationDecision moderationDecisionFromString(const std::string& s) {
    return parse(kDecisions, s, "moderation decision");
}
EnrolmentStatus enrolmentStatusFromString(const std::string& s) {
    return parse(kEnrolmentStatuses, s, "enrolment status");
}

}  // namespace lms::domain
