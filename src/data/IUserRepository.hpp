#pragma once
#include <memory>
#include <string>
#include <vector>

#include "domain/User.hpp"

namespace lms::data {

class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    // Returns nullptr when no such user exists.
    virtual std::unique_ptr<domain::User> findById(int id) const = 0;
    virtual std::unique_ptr<domain::User> findByUsername(const std::string& username) const = 0;
    // Persists a new user and returns the generated id. The id inside `user` is ignored.
    virtual int add(const domain::User& user) = 0;
    // Updates name, password hash and active flag; role and identifying numbers never change.
    virtual void update(const domain::User& user) = 0;
    virtual std::vector<std::unique_ptr<domain::User>> listAll() const = 0;
    virtual int countAll() const = 0;
};

}  // namespace lms::data
