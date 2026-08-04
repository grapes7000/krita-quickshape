#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
plugin_so="${script_dir}/kritaquickshape_krita_5_3_3.so"

if [[ ! -f "${plugin_so}" ]]; then
    echo "Error: kritaquickshape_krita_5_3_3.so not found next to this script." >&2
    exit 1
fi

install_dir="${HOME}/.local/share/krita-quickshape"
overlay_dir="${install_dir}/plugin-overlay"
bin_dir="${HOME}/.local/bin"
appimage_dir="${install_dir}/appimage"
appimage="${appimage_dir}/krita-5.3.3-x86_64.AppImage"
appimage_url="https://download.kde.org/stable/krita/5.3.3/krita-5.3.3-x86_64.AppImage"

mkdir -p "${install_dir}" "${overlay_dir}" "${bin_dir}" "${appimage_dir}"

# Download AppImage if not present
if [[ ! -f "${appimage}" ]]; then
    echo "Downloading Krita 5.3.3 AppImage..."
    curl -L -o "${appimage}" "${appimage_url}"
    chmod +x "${appimage}"
    echo "Download complete."
else
    echo "Krita 5.3.3 AppImage already present."
fi

# Extract AppImage to get bundled plugins
echo "Extracting AppImage to read bundled plugins..."
extract_dir="${install_dir}/squashfs-root"
rm -rf "${extract_dir}"
(cd "${install_dir}" && "${appimage}" --appimage-extract >/dev/null 2>&1)
bundled_plugins="${extract_dir}/usr/lib/kritaplugins"

if [[ ! -d "${bundled_plugins}" ]]; then
    echo "Error: could not find bundled plugins in extracted AppImage." >&2
    echo "Looked in: ${bundled_plugins}" >&2
    exit 1
fi

# Build plugin overlay: symlink all bundled plugins + copy QuickShape .so
echo "Setting up plugin overlay..."
find "${overlay_dir}" -maxdepth 1 -type l -delete
rm -f "${overlay_dir}/kritaquickshape_krita_5_3_3.so"

for bundled in "${bundled_plugins}"/*.so; do
    ln -s "${bundled}" "${overlay_dir}/$(basename "${bundled}")"
done
cp "${plugin_so}" "${overlay_dir}/"

# Create launcher script
launcher="${bin_dir}/krita-quickshape"
cat > "${launcher}" <<LAUNCHER
#!/usr/bin/env bash
export KRITA_PLUGIN_PATH="${overlay_dir}"
exec "${appimage}" "\$@"
LAUNCHER
chmod +x "${launcher}"

echo ""
echo "Installation complete."
echo "  Plugin overlay: ${overlay_dir}"
echo "  Launcher:       ${launcher}"
echo ""
if echo "${PATH}" | tr ':' '\n' | grep -qx "${bin_dir}"; then
    echo "Run 'krita-quickshape' to launch Krita with the QuickShape tool."
else
    echo "Add ~/.local/bin to your PATH, or run directly:"
    echo "  ${launcher}"
fi
