#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
deps_prefix="${project_dir}/build/deps/root/usr"
adapter_source="${project_dir}/adapter/krita_5_3_3"
adapter_build="${project_dir}/build/adapter-krita-5.3.3"

export CMAKE_PREFIX_PATH="${deps_prefix}"
export LD_LIBRARY_PATH="${deps_prefix}/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

cmake -S "${adapter_source}" -B "${adapter_build}" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DQt5Svg_DIR="${deps_prefix}/lib/x86_64-linux-gnu/cmake/Qt5Svg"
cmake --build "${adapter_build}" --parallel
