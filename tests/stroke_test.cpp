#include "quickshape/stroke.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

bool close(double left, double right, double tol = 1e-9) {
    return std::abs(left - right) < tol;
}

int g_failures = 0;

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++g_failures;
    }
}

quickshape::Sample make_sample(double x, double y, double pressure = 1.0,
                                std::int64_t time = 0) {
    return {.position = {x, y}, .timestamp_us = time, .pressure = pressure};
}

quickshape::Stroke make_line(double x0, double y0, double x1, double y1,
                              std::size_t n) {
    quickshape::Stroke s;
    for (std::size_t i = 0; i < n; ++i) {
        double t = (n > 1) ? static_cast<double>(i) / static_cast<double>(n - 1) : 0.0;
        s.push_back(make_sample(x0 + t * (x1 - x0), y0 + t * (y1 - y0),
                                0.2 + 0.6 * t,
                                static_cast<std::int64_t>(i) * 1000));
    }
    return s;
}

// --- Original smoothing tests ---

void test_smooth_basic() {
    quickshape::Stroke stroke{
        make_sample(0.0, 0.0, 0.2),
        make_sample(1.0, 1.0, 0.5),
        make_sample(2.0, 0.0, 0.8),
    };
    stroke[0].timestamp_us = 0;
    stroke[1].timestamp_us = 1;
    stroke[2].timestamp_us = 2;

    const auto result = quickshape::smooth_positions(stroke, {.radius = 1});
    require(result.size() == stroke.size(), "smooth: sample count changed");
    require(close(result.front().position.x, 0.0),
            "smooth: first endpoint moved");
    require(close(result.back().position.x, 2.0),
            "smooth: last endpoint moved");
    require(close(result[1].position.y, 1.0 / 3.0),
            "smooth: middle point was not averaged");
    require(close(result[1].pressure, 0.5), "smooth: pressure changed");

    const auto unchanged =
        quickshape::smooth_positions(stroke, {.radius = 0});
    require(close(unchanged[1].position.y, 1.0),
            "smooth: disabled smoothing changed stroke");
}

// --- Distance tests ---

void test_distance() {
    require(close(quickshape::distance({0, 0}, {3, 4}), 5.0),
            "distance: 3-4-5 triangle");
    require(close(quickshape::distance({1, 1}, {1, 1}), 0.0),
            "distance: same point");
}

// --- Arc length tests ---

void test_arc_length() {
    auto line = make_line(0, 0, 10, 0, 11);
    require(close(quickshape::total_arc_length(line), 10.0),
            "arc_length: horizontal line");

    auto lengths = quickshape::cumulative_arc_lengths(line);
    require(lengths.size() == 11, "arc_length: count mismatch");
    require(close(lengths[0], 0.0), "arc_length: first is zero");
    require(close(lengths[5], 5.0), "arc_length: midpoint");
    require(close(lengths[10], 10.0), "arc_length: endpoint");

    quickshape::Stroke single{make_sample(5, 5)};
    require(close(quickshape::total_arc_length(single), 0.0),
            "arc_length: single sample");
}

// --- Deduplication tests ---

void test_deduplicate() {
    quickshape::Stroke stroke{
        make_sample(0, 0), make_sample(0.1, 0), make_sample(0.2, 0),
        make_sample(5, 0), make_sample(5.1, 0), make_sample(10, 0),
    };
    auto result =
        quickshape::deduplicate(stroke, {.min_distance = 1.0});
    require(result.size() >= 2, "dedup: too few samples");
    require(close(result.front().position.x, 0.0),
            "dedup: first endpoint moved");
    require(close(result.back().position.x, 10.0),
            "dedup: last endpoint moved");

    auto dup_free =
        quickshape::deduplicate(stroke, {.min_distance = 0.0});
    require(dup_free.size() == stroke.size(),
            "dedup: zero threshold removed samples");

    quickshape::Stroke tiny{make_sample(1, 1)};
    require(quickshape::deduplicate(tiny).size() == 1,
            "dedup: single sample");
}

// --- Resample tests ---

void test_resample() {
    auto line = make_line(0, 0, 10, 0, 11);
    auto resampled = quickshape::resample_by_arc_length(
        line, {.target_count = 21});
    require(resampled.size() == 21, "resample: wrong count");
    require(close(resampled.front().position.x, 0.0),
            "resample: first endpoint moved");
    require(close(resampled.back().position.x, 10.0),
            "resample: last endpoint moved");
    require(close(resampled[10].position.x, 5.0, 0.01),
            "resample: midpoint off");

    auto by_spacing = quickshape::resample_by_arc_length(
        line, {.spacing = 2.0});
    require(by_spacing.size() >= 2, "resample: spacing produced <2 samples");
    require(close(by_spacing.front().position.x, 0.0),
            "resample: spacing first endpoint");
    require(close(by_spacing.back().position.x, 10.0),
            "resample: spacing last endpoint");

    auto pressure_check = quickshape::resample_by_arc_length(
        line, {.target_count = 11});
    require(close(pressure_check[5].pressure, 0.5, 0.01),
            "resample: pressure interpolation off");

    quickshape::Stroke single{make_sample(3, 3)};
    require(quickshape::resample_by_arc_length(single, {.target_count = 5})
                .size() == 1,
            "resample: single sample should pass through");
}

// --- Corner detection tests ---

void test_corner_detection() {
    // V-shape: sharp corner at (5,5)
    quickshape::Stroke vshape;
    for (int i = 0; i <= 10; ++i)
        vshape.push_back(make_sample(static_cast<double>(i),
                                      static_cast<double>(i)));
    for (int i = 1; i <= 10; ++i)
        vshape.push_back(make_sample(10.0 + static_cast<double>(i),
                                      10.0 - static_cast<double>(i)));

    auto corners = quickshape::detect_corners(vshape, {.angle_threshold_deg = 20.0,
                                                        .neighborhood = 3});
    bool found_corner = false;
    for (auto c : corners) {
        if (c >= 8 && c <= 12) found_corner = true;
    }
    require(found_corner, "corner: V-shape corner not detected");

    // Straight line should have no corners
    auto line = make_line(0, 0, 10, 0, 20);
    auto line_corners = quickshape::detect_corners(line);
    require(line_corners.empty(), "corner: straight line has corners");

    // Too few samples
    quickshape::Stroke tiny{make_sample(0, 0), make_sample(1, 1)};
    require(quickshape::detect_corners(tiny).empty(),
            "corner: tiny stroke has corners");
}

// --- Corner-aware smoothing tests ---

void test_smooth_preserve_corners() {
    // Zigzag with a sharp corner at index 5
    quickshape::Stroke zigzag;
    for (int i = 0; i <= 4; ++i)
        zigzag.push_back(make_sample(static_cast<double>(i),
                                      static_cast<double>(i) + 0.1 * (i % 2)));
    zigzag.push_back(make_sample(5.0, 5.0));
    for (int i = 1; i <= 4; ++i)
        zigzag.push_back(make_sample(5.0 + static_cast<double>(i),
                                      5.0 - static_cast<double>(i) + 0.1 * (i % 2)));

    std::vector<std::size_t> corners{5};
    auto result = quickshape::smooth_positions_preserve_corners(
        zigzag, corners, {.radius = 2});

    require(close(result[5].position.x, 5.0),
            "corner_smooth: corner position moved (x)");
    require(close(result[5].position.y, 5.0),
            "corner_smooth: corner position moved (y)");
    require(close(result.front().position.x, 0.0),
            "corner_smooth: first endpoint moved");
}

// --- Sensor remapping tests ---

void test_remap_sensors() {
    // Original has linearly increasing pressure 0..1
    auto original = make_line(0, 0, 10, 0, 11);
    for (std::size_t i = 0; i < original.size(); ++i)
        original[i].pressure = static_cast<double>(i) / 10.0;
    for (std::size_t i = 0; i < original.size(); ++i)
        original[i].tangential_pressure = static_cast<double>(i) / 20.0;

    // Corrected path is the same length but resampled to 21 points
    auto corrected = quickshape::resample_by_arc_length(
        original, {.target_count = 21});
    // Clear pressures on corrected
    for (auto& s : corrected) s.pressure = 0.0;

    auto remapped = quickshape::remap_sensors(corrected, original);
    require(remapped.size() == 21, "remap: wrong count");
    require(close(remapped.front().pressure, 0.0, 0.01),
            "remap: first pressure off");
    require(close(remapped.back().pressure, 1.0, 0.01),
            "remap: last pressure off");
    require(close(remapped[10].pressure, 0.5, 0.05),
            "remap: midpoint pressure off");
    require(close(remapped[10].tangential_pressure, 0.25, 0.05),
            "remap: midpoint tangential pressure off");

    // Edge cases
    quickshape::Stroke empty;
    require(quickshape::remap_sensors(empty, original).empty(),
            "remap: empty corrected");
    require(quickshape::remap_sensors(corrected, empty).size() ==
                corrected.size(),
            "remap: empty original");
}

// --- Integration: full pipeline test ---

void test_pipeline() {
    // Simulate a wobbly V-shape stroke
    quickshape::Stroke raw;
    for (int i = 0; i <= 20; ++i) {
        double x = static_cast<double>(i);
        double y = (i <= 10) ? static_cast<double>(i) : 20.0 - static_cast<double>(i);
        double jitter = 0.3 * ((i % 3) - 1);
        raw.push_back(make_sample(x, y + jitter, 0.5,
                                   static_cast<std::int64_t>(i) * 1000));
    }
    // Add some near-duplicates
    raw.insert(raw.begin() + 5,
               make_sample(raw[5].position.x + 0.01,
                           raw[5].position.y + 0.01));

    auto deduped = quickshape::deduplicate(raw, {.min_distance = 0.3});
    require(deduped.size() <= raw.size(),
            "pipeline: dedup increased size");

    auto resampled = quickshape::resample_by_arc_length(
        deduped, {.target_count = 20});
    require(resampled.size() == 20, "pipeline: resample count");

    auto corners = quickshape::detect_corners(
        resampled, {.angle_threshold_deg = 25.0, .neighborhood = 3});

    auto smoothed = quickshape::smooth_positions_preserve_corners(
        resampled, corners, {.radius = 2});
    require(smoothed.size() == 20, "pipeline: smooth count");

    auto final_stroke = quickshape::remap_sensors(smoothed, raw);
    require(final_stroke.size() == 20, "pipeline: remap count");
    require(close(final_stroke.front().position.x,
                  resampled.front().position.x),
            "pipeline: first endpoint drifted");
    require(close(final_stroke.back().position.x,
                  resampled.back().position.x),
            "pipeline: last endpoint drifted");
}

void test_gaussian_smoothing() {
    // Wobbly horizontal stroke
    quickshape::Stroke wobbly;
    for (int i = 0; i <= 30; ++i) {
        double x = static_cast<double>(i);
        double noise = 2.0 * ((i % 3) - 1.0);
        wobbly.push_back(make_sample(x, noise));
    }

    auto corners = quickshape::detect_corners(wobbly);
    auto smoothed = quickshape::smooth_gaussian(
        wobbly, corners, {.sigma = 2.0, .passes = 2});

    require(smoothed.size() == wobbly.size(),
            "gaussian: sample count changed");
    require(close(smoothed.front().position.x, 0.0),
            "gaussian: first endpoint moved");
    require(close(smoothed.back().position.x, 30.0),
            "gaussian: last endpoint moved");

    // Smoothed version should have less y-variance
    double orig_var = 0, smooth_var = 0;
    for (std::size_t i = 1; i + 1 < wobbly.size(); ++i) {
        orig_var += wobbly[i].position.y * wobbly[i].position.y;
        smooth_var += smoothed[i].position.y * smoothed[i].position.y;
    }
    require(smooth_var < orig_var,
            "gaussian: smoothing did not reduce wobble");
}

void test_adaptive_smoothing() {
    // Stroke with a curve section (high curvature) and a flat section
    quickshape::Stroke mixed;
    // Flat section
    for (int i = 0; i <= 15; ++i) {
        double noise = 1.5 * ((i % 3) - 1.0);
        mixed.push_back(make_sample(static_cast<double>(i), noise));
    }
    // Curved section (quarter circle)
    for (int i = 1; i <= 15; ++i) {
        double angle = static_cast<double>(i) / 15.0 * M_PI / 2.0;
        double noise = 1.5 * ((i % 3) - 1.0);
        mixed.push_back(
            make_sample(15.0 + 10.0 * std::sin(angle),
                        10.0 * (1.0 - std::cos(angle)) + noise));
    }

    auto corners = quickshape::detect_corners(mixed);
    auto smoothed = quickshape::smooth_adaptive(
        mixed, corners, {.base_sigma = 3.0, .passes = 2,
                         .curvature_preservation = 0.8});

    require(smoothed.size() == mixed.size(),
            "adaptive: sample count changed");
    require(close(smoothed.front().position.x, 0.0),
            "adaptive: first endpoint moved");
}

void test_gaussian_preserves_corners() {
    // V-shape with sharp corner
    quickshape::Stroke vshape;
    for (int i = 0; i <= 10; ++i)
        vshape.push_back(make_sample(static_cast<double>(i),
                                      static_cast<double>(i)));
    for (int i = 1; i <= 10; ++i)
        vshape.push_back(make_sample(10.0 + static_cast<double>(i),
                                      10.0 - static_cast<double>(i)));

    std::vector<std::size_t> corners{10};
    auto smoothed = quickshape::smooth_gaussian(
        vshape, corners, {.sigma = 2.0, .passes = 2});

    require(close(smoothed[10].position.x, 10.0, 0.01),
            "gaussian_corner: corner x moved");
    require(close(smoothed[10].position.y, 10.0, 0.01),
            "gaussian_corner: corner y moved");
}

void test_pipeline_gaussian() {
    // Full pipeline with gaussian smoothing
    quickshape::Stroke raw;
    for (int i = 0; i <= 20; ++i) {
        double x = static_cast<double>(i);
        double y = (i <= 10) ? static_cast<double>(i)
                             : 20.0 - static_cast<double>(i);
        double jitter = 0.5 * ((i % 3) - 1);
        raw.push_back(make_sample(x, y + jitter, 0.5,
                                   static_cast<std::int64_t>(i) * 1000));
    }

    auto deduped = quickshape::deduplicate(raw);
    auto resampled = quickshape::resample_by_arc_length(
        deduped, {.target_count = 30});
    auto corners = quickshape::detect_corners(resampled);
    auto smoothed = quickshape::smooth_adaptive(
        resampled, corners, {.base_sigma = 2.0, .passes = 2});
    auto final_stroke = quickshape::remap_sensors(smoothed, raw);

    require(final_stroke.size() == 30,
            "pipeline_gaussian: wrong count");
    require(close(final_stroke.front().position.x,
                  resampled.front().position.x),
            "pipeline_gaussian: first endpoint drifted");
}

}  // namespace

int main() {
    test_smooth_basic();
    test_distance();
    test_arc_length();
    test_deduplicate();
    test_resample();
    test_corner_detection();
    test_smooth_preserve_corners();
    test_remap_sensors();
    test_pipeline();
    test_gaussian_smoothing();
    test_adaptive_smoothing();
    test_gaussian_preserves_corners();
    test_pipeline_gaussian();

    if (g_failures > 0) {
        std::cerr << g_failures << " test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All stroke tests passed\n";
    return EXIT_SUCCESS;
}
