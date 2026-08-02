#include "quickshape/stroke.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

bool close(double left, double right) {
    return std::abs(left - right) < 1e-9;
}

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

}  // namespace

int main() {
    quickshape::Stroke stroke{
        {.position = {0.0, 0.0}, .timestamp_us = 0, .pressure = 0.2},
        {.position = {1.0, 1.0}, .timestamp_us = 1, .pressure = 0.5},
        {.position = {2.0, 0.0}, .timestamp_us = 2, .pressure = 0.8},
    };

    const auto result = quickshape::smooth_positions(stroke, {.radius = 1});
    require(result.size() == stroke.size(), "sample count changed");
    require(close(result.front().position.x, 0.0), "first endpoint moved");
    require(close(result.back().position.x, 2.0), "last endpoint moved");
    require(close(result[1].position.y, 1.0 / 3.0), "middle point was not averaged");
    require(close(result[1].pressure, 0.5), "pressure sample changed");

    const auto unchanged = quickshape::smooth_positions(stroke, {.radius = 0});
    require(close(unchanged[1].position.y, 1.0), "disabled smoothing changed stroke");

    std::cout << "stroke_test passed\n";
}
