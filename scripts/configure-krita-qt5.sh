#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source_dir="${project_dir}/build/krita-6.0.3-source"
build_dir="${project_dir}/build/krita-qt5-configure"
deps_prefix="${project_dir}/build/deps/root/usr"

export PATH="${deps_prefix}/bin:${PATH}"
export LD_LIBRARY_PATH="${deps_prefix}/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
export PKG_CONFIG_PATH="${deps_prefix}/lib/x86_64-linux-gnu/pkgconfig:${deps_prefix}/share/pkgconfig"

cmake -S "${source_dir}" -B "${build_dir}" \
    -DBUILD_WITH_QT6=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DCMAKE_PREFIX_PATH="${deps_prefix}" \
    -DQt5Svg_DIR="${deps_prefix}/lib/x86_64-linux-gnu/cmake/Qt5Svg" \
    -DQt5X11Extras_DIR="${deps_prefix}/lib/x86_64-linux-gnu/cmake/Qt5X11Extras" \
    -DLibExiv2_LIBRARIES="${deps_prefix}/lib/x86_64-linux-gnu/libexiv2.so" \
    -DLibExiv2_INCLUDE_DIRS="${deps_prefix}/include" \
    -DHarfBuzz_LIBRARY="${deps_prefix}/lib/x86_64-linux-gnu/libharfbuzz.so" \
    -DHarfBuzz_INCLUDE_DIR="${deps_prefix}/include/harfbuzz" \
    -Dlibunibreak_LIBRARY="${deps_prefix}/lib/x86_64-linux-gnu/libunibreak.so"
