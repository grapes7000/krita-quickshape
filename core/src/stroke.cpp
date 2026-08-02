#include "quickshape/stroke.hpp"

#include <algorithm>
#include <cmath>

namespace quickshape {

// --- Distance and arc-length utilities ---

double distance(Point a, Point b) {
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

std::vector<double> cumulative_arc_lengths(const Stroke& stroke) {
    std::vector<double> lengths;
    lengths.reserve(stroke.size());
    lengths.push_back(0.0);
    for (std::size_t i = 1; i < stroke.size(); ++i) {
        lengths.push_back(lengths.back() +
                          distance(stroke[i - 1].position, stroke[i].position));
    }
    return lengths;
}

double total_arc_length(const Stroke& stroke) {
    if (stroke.size() < 2) return 0.0;
    const auto lengths = cumulative_arc_lengths(stroke);
    return lengths.back();
}

// --- Deduplication ---

Stroke deduplicate(const Stroke& input, const DeduplicateOptions& options) {
    if (input.size() < 2) return input;

    Stroke output;
    output.reserve(input.size());
    output.push_back(input.front());

    for (std::size_t i = 1; i < input.size(); ++i) {
        if (distance(output.back().position, input[i].position) >=
            options.min_distance) {
            output.push_back(input[i]);
        }
    }

    if (distance(output.back().position, input.back().position) > 1e-12) {
        output.push_back(input.back());
    }

    return output;
}

// --- Arc-length resampling ---

namespace {

Sample lerp_sample(const Sample& a, const Sample& b, double t) {
    Sample result;
    result.position.x = a.position.x + t * (b.position.x - a.position.x);
    result.position.y = a.position.y + t * (b.position.y - a.position.y);
    result.timestamp_us =
        a.timestamp_us +
        static_cast<std::int64_t>(
            t * static_cast<double>(b.timestamp_us - a.timestamp_us));
    result.pressure = a.pressure + t * (b.pressure - a.pressure);
    result.tilt_x = a.tilt_x + t * (b.tilt_x - a.tilt_x);
    result.tilt_y = a.tilt_y + t * (b.tilt_y - a.tilt_y);
    result.rotation = a.rotation + t * (b.rotation - a.rotation);
    result.tangential_pressure =
        a.tangential_pressure +
        t * (b.tangential_pressure - a.tangential_pressure);
    return result;
}

Sample sample_at_arc_length(const Stroke& stroke,
                            const std::vector<double>& cum_lengths,
                            double target_length) {
    if (target_length <= 0.0) return stroke.front();
    if (target_length >= cum_lengths.back()) return stroke.back();

    auto it = std::lower_bound(cum_lengths.begin(), cum_lengths.end(),
                               target_length);
    auto idx = static_cast<std::size_t>(it - cum_lengths.begin());
    if (idx == 0) idx = 1;

    const double seg_start = cum_lengths[idx - 1];
    const double seg_end = cum_lengths[idx];
    const double seg_len = seg_end - seg_start;
    const double t = (seg_len > 1e-12) ? (target_length - seg_start) / seg_len
                                        : 0.0;
    return lerp_sample(stroke[idx - 1], stroke[idx], t);
}

}  // namespace

Stroke resample_by_arc_length(const Stroke& input,
                              const ResampleOptions& options) {
    if (input.size() < 2) return input;

    const auto cum_lengths = cumulative_arc_lengths(input);
    const double total = cum_lengths.back();
    if (total < 1e-12) return input;

    std::size_t count = options.target_count;
    if (count == 0 && options.spacing > 0.0) {
        count = static_cast<std::size_t>(std::round(total / options.spacing)) + 1;
        if (count < 2) count = 2;
    }
    if (count < 2) count = input.size();

    Stroke output;
    output.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const double frac =
            static_cast<double>(i) / static_cast<double>(count - 1);
        output.push_back(
            sample_at_arc_length(input, cum_lengths, frac * total));
    }

    output.front() = input.front();
    output.back() = input.back();

    return output;
}

// --- Corner detection ---

std::vector<std::size_t> detect_corners(const Stroke& input,
                                        const CornerOptions& options) {
    std::vector<std::size_t> corners;
    if (input.size() < 3 || options.neighborhood == 0) return corners;

    const double threshold_rad =
        options.angle_threshold_deg * M_PI / 180.0;

    for (std::size_t i = 1; i + 1 < input.size(); ++i) {
        const auto back_idx =
            i > options.neighborhood ? i - options.neighborhood : 0;
        const auto fwd_idx =
            std::min(input.size() - 1, i + options.neighborhood);

        const double dx1 = input[i].position.x - input[back_idx].position.x;
        const double dy1 = input[i].position.y - input[back_idx].position.y;
        const double dx2 = input[fwd_idx].position.x - input[i].position.x;
        const double dy2 = input[fwd_idx].position.y - input[i].position.y;

        const double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
        const double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);
        if (len1 < 1e-12 || len2 < 1e-12) continue;

        const double dot = (dx1 * dx2 + dy1 * dy2) / (len1 * len2);
        const double clamped = std::clamp(dot, -1.0, 1.0);
        const double turning_angle = std::acos(clamped);

        if (turning_angle >= threshold_rad) {
            corners.push_back(i);
        }
    }

    return corners;
}

// --- Smoothing ---

Stroke smooth_positions(const Stroke& input, const SmoothingOptions& options) {
    if (input.size() < 3 || options.radius == 0) {
        return input;
    }

    Stroke output = input;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (options.preserve_endpoints && (i == 0 || i + 1 == input.size())) {
            continue;
        }

        const auto begin = i > options.radius ? i - options.radius : 0;
        const auto end = std::min(input.size() - 1, i + options.radius);
        Point average{};
        for (std::size_t j = begin; j <= end; ++j) {
            average.x += input[j].position.x;
            average.y += input[j].position.y;
        }
        const auto count = static_cast<double>(end - begin + 1);
        output[i].position = {average.x / count, average.y / count};
    }
    return output;
}

Stroke smooth_positions_preserve_corners(
    const Stroke& input, const std::vector<std::size_t>& corners,
    const SmoothingOptions& options) {
    if (input.size() < 3 || options.radius == 0) {
        return input;
    }

    Stroke output = input;

    auto is_protected = [&](std::size_t idx) {
        if (options.preserve_endpoints &&
            (idx == 0 || idx + 1 == input.size())) {
            return true;
        }
        for (auto c : corners) {
            if (c == idx) return true;
        }
        return false;
    };

    for (std::size_t i = 0; i < input.size(); ++i) {
        if (is_protected(i)) continue;

        const auto begin = i > options.radius ? i - options.radius : 0;
        const auto end = std::min(input.size() - 1, i + options.radius);
        Point average{};
        double weight_sum = 0.0;
        for (std::size_t j = begin; j <= end; ++j) {
            double w = 1.0;
            for (auto c : corners) {
                if ((j <= c && c <= i) || (i <= c && c <= j)) {
                    w = 0.0;
                    break;
                }
            }
            average.x += w * input[j].position.x;
            average.y += w * input[j].position.y;
            weight_sum += w;
        }
        if (weight_sum > 0.0) {
            output[i].position = {average.x / weight_sum,
                                  average.y / weight_sum};
        }
    }
    return output;
}

// --- Sensor remapping ---

Stroke remap_sensors(const Stroke& corrected, const Stroke& original) {
    if (corrected.empty() || original.empty()) return corrected;
    if (original.size() == 1) {
        Stroke result = corrected;
        for (auto& s : result) {
            s.pressure = original[0].pressure;
            s.tilt_x = original[0].tilt_x;
            s.tilt_y = original[0].tilt_y;
            s.rotation = original[0].rotation;
            s.tangential_pressure = original[0].tangential_pressure;
        }
        return result;
    }

    const auto orig_lengths = cumulative_arc_lengths(original);
    const double orig_total = orig_lengths.back();
    const auto corr_lengths = cumulative_arc_lengths(corrected);
    const double corr_total = corr_lengths.back();

    Stroke result = corrected;
    for (std::size_t i = 0; i < corrected.size(); ++i) {
        const double norm_pos =
            (corr_total > 1e-12) ? corr_lengths[i] / corr_total : 0.0;
        const double orig_target = norm_pos * orig_total;

        const Sample mapped =
            sample_at_arc_length(original, orig_lengths, orig_target);
        result[i].pressure = mapped.pressure;
        result[i].tilt_x = mapped.tilt_x;
        result[i].tilt_y = mapped.tilt_y;
        result[i].rotation = mapped.rotation;
    }
    return result;
}

}  // namespace quickshape
