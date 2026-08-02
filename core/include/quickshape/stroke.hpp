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

struct SmoothingOptions {
    std::size_t radius{2};
    bool preserve_endpoints{true};
};

[[nodiscard]] Stroke smooth_positions(const Stroke& input,
                                      const SmoothingOptions& options = {});

}  // namespace quickshape
