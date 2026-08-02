#include "quickshape/host_contract.hpp"

#include <algorithm>

namespace quickshape {

StrokeLifecycle::StrokeLifecycle(HoldOptions options) : options_(options) {}

bool StrokeLifecycle::begin(const Sample& sample) {
    if (phase_ != StrokePhase::idle) return false;
    captured_.assign(1, sample);
    phase_ = StrokePhase::capturing;
    return true;
}

bool StrokeLifecycle::append(const Sample& sample) {
    if (phase_ != StrokePhase::capturing &&
        phase_ != StrokePhase::endpoint_held) {
        return false;
    }
    captured_.push_back(sample);
    phase_ = hold_qualifies(sample.timestamp_us) ? StrokePhase::endpoint_held
                                                 : StrokePhase::capturing;
    return true;
}

bool StrokeLifecycle::hold_qualifies(std::int64_t now_us) const {
    if (captured_.empty()) return false;

    const auto duration_us =
        std::chrono::duration_cast<std::chrono::microseconds>(options_.duration)
            .count();
    const Point endpoint = captured_.back().position;
    auto first_stationary = captured_.end();
    while (first_stationary != captured_.begin()) {
        const auto candidate = std::prev(first_stationary);
        if (distance(candidate->position, endpoint) >
            options_.maximum_endpoint_drift) {
            break;
        }
        first_stationary = candidate;
    }

    return first_stationary != captured_.end() &&
           now_us - first_stationary->timestamp_us >= duration_us;
}

bool StrokeLifecycle::begin_replay() {
    if (phase_ != StrokePhase::endpoint_held) return false;
    phase_ = StrokePhase::replaying;
    return true;
}

void StrokeLifecycle::finish() noexcept {
    captured_.clear();
    phase_ = StrokePhase::idle;
}

void StrokeLifecycle::interrupt(Interruption reason) noexcept {
    last_interruption_ = reason;
    finish();
}

StrokePhase StrokeLifecycle::phase() const noexcept { return phase_; }

const Stroke& StrokeLifecycle::captured() const noexcept { return captured_; }

Interruption StrokeLifecycle::last_interruption() const noexcept {
    return last_interruption_;
}

}  // namespace quickshape
