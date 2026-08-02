#include "quickshape/stroke.hpp"

#include <algorithm>

namespace quickshape {

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

}  // namespace quickshape
