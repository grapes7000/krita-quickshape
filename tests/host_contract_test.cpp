#include "quickshape/host_contract.hpp"

#include <cstdlib>
#include <iostream>

namespace {

int failures = 0;

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

quickshape::Sample sample(double x, std::int64_t timestamp_us) {
    return {.position = {x, 0.0}, .timestamp_us = timestamp_us};
}

void test_immediate_lift_does_not_qualify() {
    quickshape::StrokeLifecycle lifecycle;
    require(lifecycle.begin(sample(0.0, 0)), "begin immediate stroke");
    require(lifecycle.append(sample(10.0, 100'000)), "append immediate stroke");
    require(!lifecycle.hold_qualifies(100'000), "immediate lift qualified");
    lifecycle.finish();
    require(lifecycle.phase() == quickshape::StrokePhase::idle,
            "finish did not return to idle");
}

void test_stationary_hold_qualifies() {
    quickshape::StrokeLifecycle lifecycle;
    require(lifecycle.begin(sample(0.0, 0)), "begin held stroke");
    require(lifecycle.append(sample(10.0, 100'000)), "append endpoint");
    require(lifecycle.append(sample(10.5, 400'000)), "append hold sample");
    require(lifecycle.append(sample(10.25, 700'000)), "append held endpoint");
    require(lifecycle.phase() == quickshape::StrokePhase::endpoint_held,
            "stationary hold did not change phase");
    require(lifecycle.begin_replay(700'000),
            "held stroke did not enter replay");
}

void test_timer_can_qualify_without_an_extra_pointer_sample() {
    quickshape::StrokeLifecycle lifecycle;
    require(lifecycle.begin(sample(0.0, 0)), "begin timer-held stroke");
    require(lifecycle.append(sample(10.0, 100'000)),
            "append timer-held endpoint");
    require(lifecycle.phase() == quickshape::StrokePhase::capturing,
            "timer-held stroke qualified too early");
    require(lifecycle.begin_replay(700'000),
            "timer could not atomically qualify and enter replay");
    require(lifecycle.phase() == quickshape::StrokePhase::replaying,
            "timer-held stroke did not enter replay phase");
}

void test_hold_does_not_require_sample_at_exact_cutoff() {
    quickshape::StrokeLifecycle lifecycle;
    require(lifecycle.begin(sample(0.0, 0)), "begin irregular held stroke");
    require(lifecycle.append(sample(10.0, 83'000)), "append irregular endpoint");
    require(lifecycle.append(sample(10.25, 311'000)), "append irregular hold");
    require(lifecycle.append(sample(10.1, 641'000)), "append irregular held endpoint");
    require(lifecycle.hold_qualifies(700'000),
            "hold required a sample exactly at its cutoff");
}

void test_drift_rejects_hold() {
    quickshape::StrokeLifecycle lifecycle;
    require(lifecycle.begin(sample(0.0, 0)), "begin drifting stroke");
    require(lifecycle.append(sample(10.0, 100'000)), "append drift start");
    require(lifecycle.append(sample(14.0, 700'000)), "append drift end");
    require(!lifecycle.hold_qualifies(700'000), "drifting endpoint qualified");
}

void test_interruption_clears_capture() {
    quickshape::StrokeLifecycle lifecycle;
    require(lifecycle.begin(sample(0.0, 0)), "begin interrupted stroke");
    lifecycle.interrupt(quickshape::Interruption::document_closing);
    require(lifecycle.phase() == quickshape::StrokePhase::idle,
            "interruption did not return to idle");
    require(lifecycle.captured().empty(), "interruption retained samples");
    require(lifecycle.last_interruption() ==
                quickshape::Interruption::document_closing,
            "interruption reason was not retained");
}

}  // namespace

int main() {
    test_immediate_lift_does_not_qualify();
    test_stationary_hold_qualifies();
    test_timer_can_qualify_without_an_extra_pointer_sample();
    test_hold_does_not_require_sample_at_exact_cutoff();
    test_drift_rejects_hold();
    test_interruption_clears_capture();
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
