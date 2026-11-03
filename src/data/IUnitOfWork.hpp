#pragma once
#include <memory>

namespace lms::data {

// Groups several repository writes into one atomic unit. A Scope that is destroyed
// without commit() rolls everything back, so a failure can never leave half a change.
class IUnitOfWork {
public:
    class Scope {
    public:
        virtual ~Scope() = default;
        virtual void commit() = 0;
    };
    virtual ~IUnitOfWork() = default;
    virtual std::unique_ptr<Scope> begin() = 0;
};

}  // namespace lms::data
