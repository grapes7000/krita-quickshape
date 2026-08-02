#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
appimage="${project_dir}/build/krita-5.3.3-appimage/krita-5.3.3-x86_64.AppImage"
appdir_plugins="${project_dir}/build/krita-5.3.3-appimage/squashfs-root/usr/lib/kritaplugins"
adapter="${project_dir}/build/adapter-krita-5.3.3/kritaquickshape_krita_5_3_3.so"
runtime_dir="${project_dir}/build/manual-test"
plugin_overlay="${project_dir}/build/runtime/plugin-overlay"

for required_path in "${appimage}" "${appdir_plugins}" "${adapter}"; do
    if [[ ! -e "${required_path}" ]]; then
        echo "Missing required build artifact: ${required_path}" >&2
        exit 1
    fi
done

mkdir -p "${runtime_dir}"/{home,config,data,cache} "${plugin_overlay}"

# KRITA_PLUGIN_PATH replaces Krita's bundled search directory. Mirror every
# bundled plugin into the overlay so adding QuickShape does not disable Krita's
# resource and canvas plugins.
find "${plugin_overlay}" -maxdepth 1 -type l -delete
for bundled_plugin in "${appdir_plugins}"/*.so; do
    ln -s "${bundled_plugin}" "${plugin_overlay}/$(basename "${bundled_plugin}")"
done
cp "${adapter}" "${plugin_overlay}/"

export HOME="${runtime_dir}/home"
export XDG_CONFIG_HOME="${runtime_dir}/config"
export XDG_DATA_HOME="${runtime_dir}/data"
export XDG_CACHE_HOME="${runtime_dir}/cache"
export KRITA_PLUGIN_PATH="${plugin_overlay}"
export QT_OPENGL=software

exec "${appimage}" --nosplash "$@"
