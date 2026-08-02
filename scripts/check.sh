#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${project_dir}/build"

if command -v cmake >/dev/null 2>&1; then
    cmake -S "${project_dir}" -B "${build_dir}" -DCMAKE_BUILD_TYPE=Debug
    cmake --build "${build_dir}" --parallel
    ctest --test-dir "${build_dir}" --output-on-failure
elif command -v g++ >/dev/null 2>&1; then
    mkdir -p "${build_dir}"
    g++ -std=c++20 -Wall -Wextra -Wpedantic -Wconversion \
        -I"${project_dir}/core/include" \
        "${project_dir}/core/src/stroke.cpp" \
        "${project_dir}/tests/stroke_test.cpp" \
        -o "${build_dir}/stroke_test"
    "${build_dir}/stroke_test"
else
    echo "error: install CMake plus a C++20 compiler, or provide g++" >&2
    exit 1
fi
