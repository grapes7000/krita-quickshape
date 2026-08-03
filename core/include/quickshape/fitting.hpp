#pragma once

#include "quickshape/stroke.hpp"

#include <cstdint>
#include <vector>

namespace quickshape {

enum class ShapeType : std::uint8_t {
    None,
    Line,
    Circle,
    Ellipse,
    Triangle,
    Rectangle,
    Polygon,
    Star,
};

struct LineFit {
    Point start{};
    Point end{};
    double residual{};
};

struct CircleFit {
    Point center{};
    double radius{};
    double residual{};
};

struct EllipseFit {
    Point center{};
    double semi_major{};
    double semi_minor{};
    double angle_rad{};
    double residual{};
};

struct PolygonFit {
    std::vector<Point> vertices;
    double residual{};
};

struct StarFit {
    Point center{};
    double outer_radius{};
    double inner_radius{};
    double rotation_rad{};
    double residual{};
};

struct ClassifyOptions {
    double confidence_threshold{0.85};
    double closure_distance_ratio{0.10};
    double rdp_epsilon_ratio{0.02};
    double rect_angle_tolerance_deg{12.0};
};

struct ClassifyResult {
    ShapeType type{ShapeType::None};
    double confidence{};
    LineFit line;
    CircleFit circle;
    EllipseFit ellipse;
    PolygonFit polygon;
    StarFit star;
    Stroke fitted_path;
};

// --- Individual fitters ---

[[nodiscard]] LineFit fit_line(const Stroke& input);
[[nodiscard]] CircleFit fit_circle(const Stroke& input);
[[nodiscard]] EllipseFit fit_ellipse(const Stroke& input);
[[nodiscard]] StarFit fit_star(const Stroke& input);

// --- Polygon helpers ---

[[nodiscard]] std::vector<std::size_t> rdp_simplify(
    const Stroke& input, double epsilon);

[[nodiscard]] PolygonFit fit_polygon(const Stroke& input, double epsilon);

// --- Stroke generation from fits ---

[[nodiscard]] Stroke stroke_from_line(const LineFit& fit,
                                      std::size_t sample_count);
[[nodiscard]] Stroke stroke_from_circle(const CircleFit& fit,
                                        std::size_t sample_count);
[[nodiscard]] Stroke stroke_from_ellipse(const EllipseFit& fit,
                                         std::size_t sample_count);
[[nodiscard]] Stroke stroke_from_polygon(const PolygonFit& fit,
                                          std::size_t samples_per_edge);
[[nodiscard]] Stroke stroke_from_star(const StarFit& fit,
                                      std::size_t samples_per_edge);

// --- Classifier ---

[[nodiscard]] ClassifyResult classify(const Stroke& input,
                                       const ClassifyOptions& options = {});

}  // namespace quickshape
