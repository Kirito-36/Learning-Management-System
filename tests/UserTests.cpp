#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "domain/User.hpp"

using namespace lms::domain;

namespace {
// Test double: avoids the cost of Argon2 in unit tests (Dependency Inversion at work).
class FakeHasher final : public IPasswordHasher {
public:
    std::string hash(const std::string& p) const override { return "fake$" + p; }
    bool verify(const std::string& h, const std::string& p) const override { return h == "fake$" + p; }
};
const FakeHasher kHasher;
}  // namespace

TEST(User, StudentMaySubmitAndViewMarksOnly) {
    Student s(1, "Thabo", "thabo", "x", true, "S2026001");
    EXPECT_TRUE(s.can(Permission::SubmitWork));
    EXPECT_TRUE(s.can(Permission::ViewOwnMarks));
    EXPECT_FALSE(s.can(Permission::MarkSubmission));
    EXPECT_FALSE(s.can(Permission::ReleaseMarks));
}

TEST(User, LecturerMarksButCannotModerateOrRelease) {
    Lecturer l(2, "Lineo", "lineo", "x", true, "L100", 1);
    EXPECT_TRUE(l.can(Permission::MarkSubmission));
    EXPECT_TRUE(l.can(Permission::GrantExtension));
    EXPECT_FALSE(l.can(Permission::ModerateMarks));
    EXPECT_FALSE(l.can(Permission::ReleaseMarks));
}

TEST(User, HeadOfDepartmentModeratesAndReleases) {
    HeadOfDepartment h(3, "Mpho", "mpho", "x", true, "L200", 1);
    EXPECT_TRUE(h.can(Permission::ModerateMarks));
    EXPECT_TRUE(h.can(Permission::ReleaseMarks));
    EXPECT_FALSE(h.can(Permission::ManageUsers));
    EXPECT_FALSE(h.can(Permission::SubmitWork));
}

TEST(User, AdministratorManagesAccountsButNeverMarks) {
    Administrator a(4, "Admin", "admin", "x", true);
    EXPECT_TRUE(a.can(Permission::ManageUsers));
    EXPECT_TRUE(a.can(Permission::EnrolStudents));
    EXPECT_FALSE(a.can(Permission::MarkSubmission));
    EXPECT_FALSE(a.can(Permission::ReleaseMarks));
}

TEST(User, RolesAreResolvedPolymorphically) {
    std::vector<std::unique_ptr<User>> users;
    users.push_back(std::make_unique<Administrator>(1, "a", "a", "x", true));
    users.push_back(std::make_unique<Lecturer>(2, "l", "l", "x", true, "L1", 1));
    users.push_back(std::make_unique<HeadOfDepartment>(3, "h", "h", "x", true, "L2", 1));
    users.push_back(std::make_unique<Student>(4, "s", "s", "x", true, "S1"));
    const Role expected[] = {Role::Administrator, Role::Lecturer, Role::HeadOfDepartment, Role::Student};
    for (std::size_t i = 0; i < users.size(); ++i) EXPECT_EQ(users[i]->role(), expected[i]);
}

TEST(User, AuthenticationChecksPasswordAndActiveFlag) {
    Student ok(1, "s", "s", kHasher.hash("correct horse"), true, "S1");
    EXPECT_TRUE(ok.authenticate(kHasher, "correct horse"));
    EXPECT_FALSE(ok.authenticate(kHasher, "wrong"));

    Student disabled(2, "s", "s2", kHasher.hash("pw"), false, "S2");
    EXPECT_FALSE(disabled.authenticate(kHasher, "pw"));
}
