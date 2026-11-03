#pragma once
#include <utility>
#include <vector>

#include "data/IMarkRepository.hpp"
#include "data/ISubmissionRepository.hpp"
#include "domain/User.hpp"

namespace lms::services {

// Student view of results. Only RELEASED marks are ever returned: a mark that is unmarked,
// awaiting moderation or merely approved is indistinguishable from "no result yet".
class ResultsService {
public:
    ResultsService(const data::ISubmissionRepository& submissions, const data::IMarkRepository& marks)
        : submissions_(submissions), marks_(marks) {}

    std::vector<std::pair<domain::Submission, domain::Mark>> releasedFor(const domain::User& student) const;

private:
    const data::ISubmissionRepository& submissions_;
    const data::IMarkRepository& marks_;
};

}  // namespace lms::services
