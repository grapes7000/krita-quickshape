#!/usr/bin/env bash
set -euo pipefail

bundle_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
appimage="${1:-${bundle_dir}/krita-5.3.3-x86_64.AppImage}"
runtime_dir="${bundle_dir}/runtime"
appdir="${runtime_dir}/appdir"
overlay="${runtime_dir}/plugin-overlay"

if [[ ! -f "${appimage}" ]]; then
    echo "Krita 5.3.3 AppImage not found: ${appimage}" >&2
    echo "Pass its path as the first argument." >&2
    exit 1
fi

mkdir -p "${runtime_dir}"/{home,config,data,cache} "${overlay}"
if [[ ! -x "${appdir}/AppRun" ]]; then
    extraction_dir="${runtime_dir}/squashfs-root"
    rm -rf "${extraction_dir}"
    (cd "${runtime_dir}" && "${appimage}" --appimage-extract >/dev/null)
    mv "${extraction_dir}" "${appdir}"
fi

find "${overlay}" -maxdepth 1 -type l -delete
for plugin in "${appdir}"/usr/lib/kritaplugins/*.so; do
    ln -s "${plugin}" "${overlay}/$(basename "${plugin}")"
done
cp "${bundle_dir}/kritaquickshape_krita_5_3_3.so" "${overlay}/"

export HOME="${runtime_dir}/home"
export XDG_CONFIG_HOME="${runtime_dir}/config"
export XDG_DATA_HOME="${runtime_dir}/data"
export XDG_CACHE_HOME="${runtime_dir}/cache"
export KRITA_PLUGIN_PATH="${overlay}"
export QT_OPENGL=software

exec "${appdir}/AppRun" --nosplash
