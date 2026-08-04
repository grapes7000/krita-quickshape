#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
adapter="${project_dir}/build/adapter-krita-5.3.3/kritaquickshape_krita_5_3_3.so"

if [[ ! -f "${adapter}" ]]; then
    echo "Plugin .so not found. Build it first:" >&2
    echo "  ./scripts/build-krita-adapter.sh" >&2
    exit 1
fi

version="$(date +%Y%m%d)"
out_dir="${project_dir}/build/package"
staging="${out_dir}/krita-quickshape"
tarball="${out_dir}/krita-quickshape-${version}.tar.gz"

rm -rf "${staging}"
mkdir -p "${staging}"
cp "${adapter}" "${staging}/"
cp "${project_dir}/scripts/install-plugin.sh" "${staging}/"

tar -czf "${tarball}" -C "${out_dir}" krita-quickshape
rm -rf "${staging}"

echo "Package created: ${tarball}"
echo "Copy to target machine, extract, and run:"
echo "  tar xzf krita-quickshape-${version}.tar.gz"
echo "  cd krita-quickshape && ./install-plugin.sh"
