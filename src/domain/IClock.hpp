#pragma once
#include "domain/Types.hpp"

namespace lms::domain {

// Injected so deadline and penalty logic can be tested with a controllable clock.
class IClock {
public:
    virtual ~IClock() = default;
    virtual Timestamp now() const = 0;
};

}  // namespace lms::domain
