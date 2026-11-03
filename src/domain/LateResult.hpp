#pragma once
#include <algorithm>
#include <cmath>

namespace lms::domain {

// Outcome of evaluating a submission time against an effective deadline.
struct LateResult {
    bool isLate = false;
    int daysLate = 0;              // whole days, rounded up
    double penaltyFraction = 0.0;  // fraction of the assignment's maximum mark to deduct
    bool zeroMark = false;         // true when lateness exceeds the permitted window

    // Applies the penalty to a raw mark, never going below zero. Rounded to 2 d.p.
    double applyTo(double rawMark, double maxMark) const {
        if (zeroMark) return 0.0;
        const double result = std::max(0.0, rawMark - penaltyFraction * maxMark);
        return std::round(result * 100.0) / 100.0;
    }
};

}  // namespace lms::domain
