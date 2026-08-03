#include "quickshape/fitting.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace quickshape {

namespace {

Point centroid(const Stroke& s) {
    Point c{};
    for (const auto& p : s) {
        c.x += p.position.x;
        c.y += p.position.y;
    }
    const auto n = static_cast<double>(s.size());
    c.x /= n;
    c.y /= n;
    return c;
}

double point_to_segment_distance(Point p, Point a, Point b) {
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    const double len_sq = dx * dx + dy * dy;
    if (len_sq < 1e-24) return distance(p, a);
    double t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / len_sq;
    t = std::clamp(t, 0.0, 1.0);
    Point proj{a.x + t * dx, a.y + t * dy};
    return distance(p, proj);
}

double mean_distance_to_segments(const Stroke& s,
                                  const std::vector<Point>& verts,
                                  bool closed) {
    if (verts.size() < 2) return std::numeric_limits<double>::max();
    double sum = 0.0;
    for (const auto& sample : s) {
        double best = std::numeric_limits<double>::max();
        for (std::size_t i = 0; i + 1 < verts.size(); ++i) {
            best = std::min(best,
                            point_to_segment_distance(sample.position,
                                                      verts[i], verts[i + 1]));
        }
        if (closed && verts.size() >= 3) {
            best = std::min(best,
                            point_to_segment_distance(sample.position,
                                                      verts.back(),
                                                      verts.front()));
        }
        sum += best;
    }
    return sum / static_cast<double>(s.size());
}

double bounding_diagonal(const Stroke& s) {
    double xmin = s[0].position.x, xmax = xmin;
    double ymin = s[0].position.y, ymax = ymin;
    for (const auto& p : s) {
        xmin = std::min(xmin, p.position.x);
        xmax = std::max(xmax, p.position.x);
        ymin = std::min(ymin, p.position.y);
        ymax = std::max(ymax, p.position.y);
    }
    return std::sqrt((xmax - xmin) * (xmax - xmin) +
                     (ymax - ymin) * (ymax - ymin));
}

bool is_closed(const Stroke& s, double ratio) {
    if (s.size() < 3) return false;
    double diag = bounding_diagonal(s);
    if (diag < 1e-12) return true;
    return distance(s.front().position, s.back().position) / diag < ratio;
}

double angle_between_vectors(Point v1, Point v2) {
    double len1 = std::sqrt(v1.x * v1.x + v1.y * v1.y);
    double len2 = std::sqrt(v2.x * v2.x + v2.y * v2.y);
    if (len1 < 1e-12 || len2 < 1e-12) return 0.0;
    double dot = (v1.x * v2.x + v1.y * v2.y) / (len1 * len2);
    return std::acos(std::clamp(dot, -1.0, 1.0));
}

}  // namespace

// --- Line fitting (PCA) ---

LineFit fit_line(const Stroke& input) {
    LineFit result;
    if (input.size() < 2) return result;

    Point c = centroid(input);

    double cxx = 0, cxy = 0, cyy = 0;
    for (const auto& s : input) {
        double dx = s.position.x - c.x;
        double dy = s.position.y - c.y;
        cxx += dx * dx;
        cxy += dx * dy;
        cyy += dy * dy;
    }

    double trace = cxx + cyy;
    double det = cxx * cyy - cxy * cxy;
    double disc = trace * trace / 4.0 - det;
    if (disc < 0) disc = 0;
    double lambda1 = trace / 2.0 + std::sqrt(disc);

    double dir_x, dir_y;
    if (std::abs(cxy) > 1e-12) {
        dir_x = lambda1 - cyy;
        dir_y = cxy;
    } else if (cxx >= cyy) {
        dir_x = 1.0;
        dir_y = 0.0;
    } else {
        dir_x = 0.0;
        dir_y = 1.0;
    }
    double dir_len = std::sqrt(dir_x * dir_x + dir_y * dir_y);
    if (dir_len > 1e-12) {
        dir_x /= dir_len;
        dir_y /= dir_len;
    }

    double t_min = std::numeric_limits<double>::max();
    double t_max = std::numeric_limits<double>::lowest();
    for (const auto& s : input) {
        double t = (s.position.x - c.x) * dir_x +
                   (s.position.y - c.y) * dir_y;
        t_min = std::min(t_min, t);
        t_max = std::max(t_max, t);
    }

    result.start = {c.x + t_min * dir_x, c.y + t_min * dir_y};
    result.end = {c.x + t_max * dir_x, c.y + t_max * dir_y};

    double err_sum = 0;
    for (const auto& s : input) {
        err_sum += point_to_segment_distance(s.position,
                                              result.start, result.end);
    }
    result.residual = err_sum / static_cast<double>(input.size());

    return result;
}

// --- Circle fitting (Kåsa algebraic) ---

CircleFit fit_circle(const Stroke& input) {
    CircleFit result;
    if (input.size() < 3) return result;

    double sx = 0, sy = 0, sx2 = 0, sy2 = 0, sxy = 0;
    double sx3 = 0, sy3 = 0, sx2y = 0, sxy2 = 0;
    const auto n = static_cast<double>(input.size());

    for (const auto& s : input) {
        double x = s.position.x, y = s.position.y;
        sx += x;   sy += y;
        sx2 += x * x;  sy2 += y * y;  sxy += x * y;
        sx3 += x * x * x;  sy3 += y * y * y;
        sx2y += x * x * y;  sxy2 += x * y * y;
    }

    double a11 = 2.0 * (sx * sx - n * sx2);
    double a12 = 2.0 * (sx * sy - n * sxy);
    double a21 = a12;
    double a22 = 2.0 * (sy * sy - n * sy2);

    double b1 = sx * sx2 + sx * sy2 - n * (sx3 + sxy2);
    double b2 = sy * sx2 + sy * sy2 - n * (sx2y + sy3);

    double det = a11 * a22 - a12 * a21;
    if (std::abs(det) < 1e-20) {
        result.residual = std::numeric_limits<double>::max();
        return result;
    }

    result.center.x = (b1 * a22 - b2 * a12) / det;
    result.center.y = (a11 * b2 - a21 * b1) / det;

    double r_sum = 0;
    for (const auto& s : input)
        r_sum += distance(s.position, result.center);
    result.radius = r_sum / n;

    double err_sum = 0;
    for (const auto& s : input) {
        double d = std::abs(distance(s.position, result.center) - result.radius);
        err_sum += d;
    }
    result.residual = err_sum / n;

    return result;
}

// --- Ellipse fitting (covariance axes) ---

EllipseFit fit_ellipse(const Stroke& input) {
    EllipseFit result;
    if (input.size() < 5) return result;

    Point c = centroid(input);
    result.center = c;

    double cxx = 0, cxy = 0, cyy = 0;
    for (const auto& s : input) {
        double dx = s.position.x - c.x;
        double dy = s.position.y - c.y;
        cxx += dx * dx;
        cxy += dx * dy;
        cyy += dy * dy;
    }
    const auto n = static_cast<double>(input.size());
    cxx /= n;  cxy /= n;  cyy /= n;

    double trace = cxx + cyy;
    double det = cxx * cyy - cxy * cxy;
    double disc = trace * trace / 4.0 - det;
    if (disc < 0) disc = 0;
    double lambda1 = trace / 2.0 + std::sqrt(disc);
    double lambda2 = trace / 2.0 - std::sqrt(disc);
    if (lambda2 < 0) lambda2 = 0;

    double dir_x, dir_y;
    if (std::abs(cxy) > 1e-12) {
        dir_x = lambda1 - cyy;
        dir_y = cxy;
    } else if (cxx >= cyy) {
        dir_x = 1.0;
        dir_y = 0.0;
    } else {
        dir_x = 0.0;
        dir_y = 1.0;
    }
    double dir_len = std::sqrt(dir_x * dir_x + dir_y * dir_y);
    if (dir_len > 1e-12) {
        dir_x /= dir_len;
        dir_y /= dir_len;
    }

    result.angle_rad = std::atan2(dir_y, dir_x);

    double sum_a = 0, sum_b = 0;
    for (const auto& s : input) {
        double dx = s.position.x - c.x;
        double dy = s.position.y - c.y;
        double proj_major = dx * dir_x + dy * dir_y;
        double proj_minor = -dx * dir_y + dy * dir_x;
        sum_a += std::abs(proj_major);
        sum_b += std::abs(proj_minor);
    }
    result.semi_major = (sum_a / n) * (M_PI / 2.0);
    result.semi_minor = (sum_b / n) * (M_PI / 2.0);

    if (result.semi_major < result.semi_minor) {
        std::swap(result.semi_major, result.semi_minor);
        result.angle_rad += M_PI / 2.0;
    }

    double err_sum = 0;
    for (const auto& s : input) {
        double dx = s.position.x - c.x;
        double dy = s.position.y - c.y;
        double u = dx * std::cos(-result.angle_rad) -
                   dy * std::sin(-result.angle_rad);
        double v = dx * std::sin(-result.angle_rad) +
                   dy * std::cos(-result.angle_rad);
        double a = result.semi_major;
        double b = result.semi_minor;
        if (a < 1e-12 || b < 1e-12) {
            err_sum += std::sqrt(dx * dx + dy * dy);
            continue;
        }
        double norm = std::sqrt((u * u) / (a * a) + (v * v) / (b * b));
        if (norm < 1e-12) continue;
        double closest_x = u / norm;
        double closest_y = v / norm;
        double ex = u - closest_x;
        double ey = v - closest_y;
        err_sum += std::sqrt(ex * ex + ey * ey);
    }
    result.residual = err_sum / n;

    return result;
}

// --- RDP simplification ---

namespace {

void rdp_recurse(const Stroke& input, std::size_t start, std::size_t end,
                 double epsilon, std::vector<bool>& keep) {
    if (end <= start + 1) return;

    double max_dist = 0;
    std::size_t max_idx = start;
    for (std::size_t i = start + 1; i < end; ++i) {
        double d = point_to_segment_distance(
            input[i].position, input[start].position, input[end].position);
        if (d > max_dist) {
            max_dist = d;
            max_idx = i;
        }
    }

    if (max_dist > epsilon) {
        keep[max_idx] = true;
        rdp_recurse(input, start, max_idx, epsilon, keep);
        rdp_recurse(input, max_idx, end, epsilon, keep);
    }
}

}  // namespace

std::vector<std::size_t> rdp_simplify(const Stroke& input, double epsilon) {
    if (input.size() < 2)
        return input.empty() ? std::vector<std::size_t>{}
                             : std::vector<std::size_t>{0};

    std::vector<bool> keep(input.size(), false);
    keep[0] = true;
    keep[input.size() - 1] = true;
    rdp_recurse(input, 0, input.size() - 1, epsilon, keep);

    std::vector<std::size_t> indices;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (keep[i]) indices.push_back(i);
    }
    return indices;
}

// --- Polygon fitting ---

PolygonFit fit_polygon(const Stroke& input, double epsilon) {
    PolygonFit result;
    if (input.size() < 3) return result;

    auto indices = rdp_simplify(input, epsilon);
    for (auto i : indices)
        result.vertices.push_back(input[i].position);

    bool closed = is_closed(input, 0.15);
    if (closed && result.vertices.size() >= 3) {
        if (distance(result.vertices.front(), result.vertices.back()) <
            epsilon * 2.0) {
            result.vertices.pop_back();
        }
    }

    result.residual =
        mean_distance_to_segments(input, result.vertices, closed);
    return result;
}

// --- Stroke generation ---

Stroke stroke_from_line(const LineFit& fit, std::size_t sample_count) {
    Stroke s;
    if (sample_count < 2) sample_count = 2;
    s.reserve(sample_count);
    for (std::size_t i = 0; i < sample_count; ++i) {
        double t = static_cast<double>(i) /
                   static_cast<double>(sample_count - 1);
        Sample sam;
        sam.position.x = fit.start.x + t * (fit.end.x - fit.start.x);
        sam.position.y = fit.start.y + t * (fit.end.y - fit.start.y);
        s.push_back(sam);
    }
    return s;
}

Stroke stroke_from_circle(const CircleFit& fit, std::size_t sample_count) {
    Stroke s;
    if (sample_count < 4) sample_count = 64;
    s.reserve(sample_count);
    for (std::size_t i = 0; i < sample_count; ++i) {
        double t = static_cast<double>(i) /
                   static_cast<double>(sample_count);
        double angle = 2.0 * M_PI * t;
        Sample sam;
        sam.position.x = fit.center.x + fit.radius * std::cos(angle);
        sam.position.y = fit.center.y + fit.radius * std::sin(angle);
        s.push_back(sam);
    }
    return s;
}

Stroke stroke_from_ellipse(const EllipseFit& fit, std::size_t sample_count) {
    Stroke s;
    if (sample_count < 4) sample_count = 64;
    s.reserve(sample_count);
    double ca = std::cos(fit.angle_rad);
    double sa = std::sin(fit.angle_rad);
    for (std::size_t i = 0; i < sample_count; ++i) {
        double t = static_cast<double>(i) /
                   static_cast<double>(sample_count);
        double angle = 2.0 * M_PI * t;
        double u = fit.semi_major * std::cos(angle);
        double v = fit.semi_minor * std::sin(angle);
        Sample sam;
        sam.position.x = fit.center.x + u * ca - v * sa;
        sam.position.y = fit.center.y + u * sa + v * ca;
        s.push_back(sam);
    }
    return s;
}

Stroke stroke_from_polygon(const PolygonFit& fit,
                            std::size_t samples_per_edge) {
    Stroke s;
    if (fit.vertices.size() < 2) return s;
    if (samples_per_edge < 2) samples_per_edge = 8;

    auto add_edge = [&](Point a, Point b) {
        std::size_t start_idx = s.empty() ? 0 : 1;
        for (std::size_t i = start_idx; i < samples_per_edge; ++i) {
            double t = static_cast<double>(i) /
                       static_cast<double>(samples_per_edge - 1);
            Sample sam;
            sam.position.x = a.x + t * (b.x - a.x);
            sam.position.y = a.y + t * (b.y - a.y);
            s.push_back(sam);
        }
    };

    for (std::size_t i = 0; i + 1 < fit.vertices.size(); ++i)
        add_edge(fit.vertices[i], fit.vertices[i + 1]);
    if (fit.vertices.size() >= 3)
        add_edge(fit.vertices.back(), fit.vertices.front());

    return s;
}

// --- Classifier ---

ClassifyResult classify(const Stroke& input, const ClassifyOptions& options) {
    ClassifyResult best;
    best.type = ShapeType::None;
    best.confidence = 0.0;

    if (input.size() < 2) return best;

    const double diag = bounding_diagonal(input);
    if (diag < 1e-12) return best;

    const bool closed = is_closed(input, options.closure_distance_ratio);
    const double epsilon = diag * options.rdp_epsilon_ratio;
    const double rect_tol_rad =
        options.rect_angle_tolerance_deg * M_PI / 180.0;

    auto score = [&](double residual) -> double {
        double normalized = residual / diag;
        return std::max(0.0, 1.0 - normalized * 20.0);
    };

    // Line (open strokes only)
    if (!closed) {
        auto lf = fit_line(input);
        double conf = score(lf.residual);
        if (conf > best.confidence) {
            best.type = ShapeType::Line;
            best.confidence = conf;
            best.line = lf;
            best.fitted_path = stroke_from_line(lf, input.size());
        }
    }

    // Circle (closed strokes)
    if (closed) {
        auto cf = fit_circle(input);
        if (cf.radius > 1e-6) {
            double conf = score(cf.residual);
            if (conf > best.confidence) {
                best.type = ShapeType::Circle;
                best.confidence = conf;
                best.circle = cf;
                best.fitted_path = stroke_from_circle(cf, input.size());
            }
        }
    }

    // Ellipse (closed strokes, only if it beats circle)
    if (closed) {
        auto ef = fit_ellipse(input);
        if (ef.semi_major > 1e-6 && ef.semi_minor > 1e-6) {
            double aspect = ef.semi_minor / ef.semi_major;
            if (aspect < 0.95) {
                double conf = score(ef.residual);
                if (conf > best.confidence) {
                    best.type = ShapeType::Ellipse;
                    best.confidence = conf;
                    best.ellipse = ef;
                    best.fitted_path =
                        stroke_from_ellipse(ef, input.size());
                }
            }
        }
    }

    // Polygon (closed strokes)
    if (closed) {
        auto pf = fit_polygon(input, epsilon);
        std::size_t nv = pf.vertices.size();
        if (nv >= 3) {
            double conf = score(pf.residual);

            ShapeType poly_type = ShapeType::Polygon;

            if (nv == 3) {
                poly_type = ShapeType::Triangle;
            } else if (nv == 4) {
                bool is_rect = true;
                for (std::size_t i = 0; i < 4; ++i) {
                    Point v1{pf.vertices[(i + 1) % 4].x - pf.vertices[i].x,
                             pf.vertices[(i + 1) % 4].y - pf.vertices[i].y};
                    Point v2{pf.vertices[(i + 2) % 4].x -
                                 pf.vertices[(i + 1) % 4].x,
                             pf.vertices[(i + 2) % 4].y -
                                 pf.vertices[(i + 1) % 4].y};
                    double a = angle_between_vectors(v1, v2);
                    if (std::abs(a - M_PI / 2.0) > rect_tol_rad) {
                        is_rect = false;
                        break;
                    }
                }
                if (is_rect) poly_type = ShapeType::Rectangle;
            }

            if (conf > best.confidence) {
                best.type = poly_type;
                best.confidence = conf;
                best.polygon = pf;
                best.fitted_path =
                    stroke_from_polygon(pf, input.size() / nv + 1);
            }
        }
    }

    if (best.confidence < options.confidence_threshold) {
        best.type = ShapeType::None;
    }

    return best;
}

}  // namespace quickshape
