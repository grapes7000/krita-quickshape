#pragma once

#include <cstdint>
#include <vector>

namespace quickshape {

struct Point {
    double x{};
    double y{};
};

struct Sample {
    Point position{};
    std::int64_t timestamp_us{};
    double pressure{1.0};
    double tilt_x{};
    double tilt_y{};
    double rotation{};
};

using Stroke = std::vector<Sample>;

// --- Distance and arc-length utilities ---

[[nodiscard]] double distance(Point a, Point b);

[[nodiscard]] std::vector<double> cumulative_arc_lengths(const Stroke& stroke);

[[nodiscard]] double total_arc_length(const Stroke& stroke);

// --- Deduplication ---

struct DeduplicateOptions {
    double min_distance{0.5};
};

[[nodiscard]] Stroke deduplicate(const Stroke& input,
                                 const DeduplicateOptions& options = {});

// --- Arc-length resampling ---

struct ResampleOptions {
    std::size_t target_count{0};
    double spacing{0.0};
};

[[nodiscard]] Stroke resample_by_arc_length(const Stroke& input,
                                            const ResampleOptions& options = {});

// --- Corner detection ---

struct CornerOptions {
    double angle_threshold_deg{30.0};
    std::size_t neighborhood{3};
};

[[nodiscard]] std::vector<std::size_t> detect_corners(
    const Stroke& input, const CornerOptions& options = {});

// --- Smoothing ---

struct SmoothingOptions {
    std::size_t radius{2};
    bool preserve_endpoints{true};
};

[[nodiscard]] Stroke smooth_positions(const Stroke& input,
                                      const SmoothingOptions& options = {});

[[nodiscard]] Stroke smooth_positions_preserve_corners(
    const Stroke& input,
    const std::vector<std::size_t>& corners,
    const SmoothingOptions& options = {});

// --- Sensor remapping ---

[[nodiscard]] Stroke remap_sensors(const Stroke& corrected,
                                   const Stroke& original);

}  // namespace quickshape
