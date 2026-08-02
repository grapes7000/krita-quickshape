#pragma once

#include "quickshape/stroke.hpp"

#include <chrono>
#include <cstdint>
#include <span>

namespace quickshape {

enum class Interruption {
    escape,
    tool_deactivated,
    node_changed,
    document_closing,
    unsupported_node,
    replay_failed,
};

// Version-neutral boundary implemented by a host-specific adapter. A transaction
// remains cancellable until end_active_brush_transaction() succeeds.
class HostAdapter {
public:
    virtual ~HostAdapter() = default;

    [[nodiscard]] virtual bool begin_active_brush_transaction(
        const Sample& first_sample) = 0;
    [[nodiscard]] virtual bool append_active_brush_samples(
        std::span<const Sample> samples) = 0;
    [[nodiscard]] virtual bool end_active_brush_transaction() = 0;
    virtual void cancel_active_brush_transaction() noexcept = 0;
};

enum class StrokePhase {
    idle,
    capturing,
    endpoint_held,
    replaying,
};

struct HoldOptions {
    std::chrono::milliseconds duration{600};
    double maximum_endpoint_drift{2.0};
};

class StrokeLifecycle {
public:
    explicit StrokeLifecycle(HoldOptions options = {});

    [[nodiscard]] bool begin(const Sample& sample);
    [[nodiscard]] bool append(const Sample& sample);
    [[nodiscard]] bool hold_qualifies(std::int64_t now_us) const;
    [[nodiscard]] bool begin_replay(std::int64_t now_us);
    void finish() noexcept;
    void interrupt(Interruption reason) noexcept;

    [[nodiscard]] StrokePhase phase() const noexcept;
    [[nodiscard]] const Stroke& captured() const noexcept;
    [[nodiscard]] Interruption last_interruption() const noexcept;

private:
    HoldOptions options_;
    Stroke captured_;
    StrokePhase phase_{StrokePhase::idle};
    Interruption last_interruption_{Interruption::escape};
};

}  // namespace quickshape
