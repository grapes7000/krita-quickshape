#include "quickshape/fitting.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

int g_failures = 0;

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++g_failures;
    }
}

bool close(double a, double b, double tol = 0.1) {
    return std::abs(a - b) < tol;
}

quickshape::Sample make_sample(double x, double y) {
    return {.position = {x, y}};
}

quickshape::Stroke make_circle(double cx, double cy, double r,
                                std::size_t n, double jitter = 0.0) {
    quickshape::Stroke s;
    for (std::size_t i = 0; i < n; ++i) {
        double t = static_cast<double>(i) / static_cast<double>(n);
        double angle = 2.0 * M_PI * t;
        double noise = jitter * ((static_cast<double>(i % 5) - 2.0) / 2.0);
        s.push_back(make_sample(cx + (r + noise) * std::cos(angle),
                                cy + (r + noise) * std::sin(angle)));
    }
    s.push_back(s.front());
    return s;
}

quickshape::Stroke make_ellipse(double cx, double cy, double a, double b,
                                 double angle_rad, std::size_t n) {
    quickshape::Stroke s;
    double ca = std::cos(angle_rad), sa = std::sin(angle_rad);
    for (std::size_t i = 0; i < n; ++i) {
        double t = static_cast<double>(i) / static_cast<double>(n);
        double theta = 2.0 * M_PI * t;
        double u = a * std::cos(theta);
        double v = b * std::sin(theta);
        s.push_back(make_sample(cx + u * ca - v * sa,
                                cy + u * sa + v * ca));
    }
    s.push_back(s.front());
    return s;
}

quickshape::Stroke make_rectangle(double cx, double cy, double w, double h,
                                   std::size_t per_side) {
    quickshape::Stroke s;
    auto add_edge = [&](double x0, double y0, double x1, double y1) {
        for (std::size_t i = 0; i < per_side; ++i) {
            double t = static_cast<double>(i) /
                       static_cast<double>(per_side);
            s.push_back(make_sample(x0 + t * (x1 - x0),
                                    y0 + t * (y1 - y0)));
        }
    };
    double x0 = cx - w / 2, y0 = cy - h / 2;
    double x1 = cx + w / 2, y1 = cy + h / 2;
    add_edge(x0, y0, x1, y0);
    add_edge(x1, y0, x1, y1);
    add_edge(x1, y1, x0, y1);
    add_edge(x0, y1, x0, y0);
    s.push_back(s.front());
    return s;
}

quickshape::Stroke make_triangle(double cx, double cy, double r,
                                  std::size_t per_side) {
    quickshape::Stroke s;
    quickshape::Point verts[3];
    for (int i = 0; i < 3; ++i) {
        double angle = 2.0 * M_PI * static_cast<double>(i) / 3.0 - M_PI / 2.0;
        verts[i] = {cx + r * std::cos(angle), cy + r * std::sin(angle)};
    }
    for (int e = 0; e < 3; ++e) {
        auto& a = verts[e];
        auto& b = verts[(e + 1) % 3];
        for (std::size_t i = 0; i < per_side; ++i) {
            double t = static_cast<double>(i) /
                       static_cast<double>(per_side);
            s.push_back(make_sample(a.x + t * (b.x - a.x),
                                    a.y + t * (b.y - a.y)));
        }
    }
    s.push_back(s.front());
    return s;
}

// --- Tests ---

void test_fit_line() {
    quickshape::Stroke line;
    for (int i = 0; i <= 20; ++i)
        line.push_back(make_sample(static_cast<double>(i),
                                    2.0 * static_cast<double>(i) + 0.1 * (i % 3 - 1)));

    auto lf = quickshape::fit_line(line);
    require(lf.residual < 1.0, "fit_line: residual too large");
    double fit_len = quickshape::distance(lf.start, lf.end);
    require(fit_len > 15.0, "fit_line: fitted line too short");
}

void test_fit_circle() {
    auto circle = make_circle(50, 50, 30, 60);
    auto cf = quickshape::fit_circle(circle);
    require(close(cf.center.x, 50, 2), "fit_circle: center.x off");
    require(close(cf.center.y, 50, 2), "fit_circle: center.y off");
    require(close(cf.radius, 30, 2), "fit_circle: radius off");
    require(cf.residual < 2.0, "fit_circle: residual too large");
}

void test_fit_circle_with_jitter() {
    auto circle = make_circle(100, 100, 50, 80, 3.0);
    auto cf = quickshape::fit_circle(circle);
    require(close(cf.center.x, 100, 5), "fit_circle_jitter: center.x off");
    require(close(cf.center.y, 100, 5), "fit_circle_jitter: center.y off");
    require(close(cf.radius, 50, 5), "fit_circle_jitter: radius off");
}

void test_fit_ellipse() {
    auto ellipse = make_ellipse(50, 50, 40, 20, 0.0, 80);
    auto ef = quickshape::fit_ellipse(ellipse);
    require(close(ef.center.x, 50, 3), "fit_ellipse: center.x off");
    require(close(ef.center.y, 50, 3), "fit_ellipse: center.y off");
    require(ef.semi_major > ef.semi_minor,
            "fit_ellipse: major < minor");
    require(ef.residual < 5.0, "fit_ellipse: residual too large");
}

void test_rdp_simplify() {
    quickshape::Stroke line;
    for (int i = 0; i <= 100; ++i)
        line.push_back(make_sample(static_cast<double>(i), 0.0));

    auto indices = quickshape::rdp_simplify(line, 0.5);
    require(indices.size() == 2, "rdp: straight line should simplify to 2");
    require(indices.front() == 0, "rdp: first index");
    require(indices.back() == 100, "rdp: last index");
}

void test_fit_polygon_triangle() {
    auto tri = make_triangle(50, 50, 30, 20);
    auto pf = quickshape::fit_polygon(tri, 2.0);
    require(pf.vertices.size() == 3 || pf.vertices.size() == 4,
            "fit_polygon_tri: wrong vertex count");
}

void test_classify_line() {
    quickshape::Stroke line;
    for (int i = 0; i <= 30; ++i)
        line.push_back(make_sample(static_cast<double>(i), 0.5 * static_cast<double>(i)));

    auto result = quickshape::classify(line, {.confidence_threshold = 0.5});
    require(result.type == quickshape::ShapeType::Line,
            "classify_line: not recognized as line");
    require(result.confidence > 0.5, "classify_line: low confidence");
}

void test_classify_circle() {
    auto circle = make_circle(50, 50, 30, 80);
    auto result = quickshape::classify(circle, {.confidence_threshold = 0.5});
    require(result.type == quickshape::ShapeType::Circle ||
                result.type == quickshape::ShapeType::Ellipse ||
                result.type == quickshape::ShapeType::Polygon,
            "classify_circle: not recognized");
    require(result.confidence > 0.5, "classify_circle: low confidence");
}

void test_classify_rectangle() {
    auto rect = make_rectangle(50, 50, 40, 20, 15);
    auto result = quickshape::classify(rect, {.confidence_threshold = 0.3});
    require(result.type == quickshape::ShapeType::Rectangle ||
                result.type == quickshape::ShapeType::Polygon ||
                result.type == quickshape::ShapeType::Triangle,
            "classify_rect: not recognized as polygon-like shape");
    require(result.confidence > 0.3, "classify_rect: low confidence");
}

void test_classify_fallback() {
    quickshape::Stroke chaotic;
    for (int i = 0; i < 30; ++i) {
        double angle = static_cast<double>(i) * 0.7;
        double r = 10.0 + 8.0 * std::sin(static_cast<double>(i) * 1.3);
        chaotic.push_back(make_sample(r * std::cos(angle),
                                       r * std::sin(angle)));
    }

    auto result =
        quickshape::classify(chaotic, {.confidence_threshold = 0.85});
    require(result.type == quickshape::ShapeType::None,
            "classify_fallback: chaotic stroke was classified");
}

void test_stroke_generation() {
    auto line_stroke = quickshape::stroke_from_line(
        {{0, 0}, {10, 10}, 0}, 10);
    require(line_stroke.size() == 10, "gen_line: wrong count");
    require(close(line_stroke.front().position.x, 0, 0.01),
            "gen_line: start.x");
    require(close(line_stroke.back().position.x, 10, 0.01),
            "gen_line: end.x");

    auto circle_stroke = quickshape::stroke_from_circle(
        {{50, 50}, 30, 0}, 32);
    require(circle_stroke.size() == 32, "gen_circle: wrong count");

    quickshape::PolygonFit pf;
    pf.vertices = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
    auto poly_stroke = quickshape::stroke_from_polygon(pf, 5);
    require(poly_stroke.size() > 10, "gen_polygon: too few samples");
}

}  // namespace

int main() {
    test_fit_line();
    test_fit_circle();
    test_fit_circle_with_jitter();
    test_fit_ellipse();
    test_rdp_simplify();
    test_fit_polygon_triangle();
    test_classify_line();
    test_classify_circle();
    test_classify_rectangle();
    test_classify_fallback();
    test_stroke_generation();

    if (g_failures > 0) {
        std::cerr << g_failures << " fitting test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All fitting tests passed\n";
    return EXIT_SUCCESS;
}
