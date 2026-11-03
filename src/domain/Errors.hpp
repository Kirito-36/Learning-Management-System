#pragma once
#include <stdexcept>

namespace lms::domain {

// Base class for every error the application raises deliberately. The UI catches
// LmsError and shows e.what() to the user; anything else is treated as a fault.
class LmsError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
class ValidationError : public LmsError { public: using LmsError::LmsError; };
class AccessDenied : public LmsError { public: using LmsError::LmsError; };
class NotFound : public LmsError { public: using LmsError::LmsError; };
class AuthenticationError : public LmsError { public: using LmsError::LmsError; };
class RuleViolation : public LmsError { public: using LmsError::LmsError; };

}  // namespace lms::domain
