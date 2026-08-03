# Read-only Feasibility Checklist

Complete this before integration code or dependency installation.

Audit target: official `krita-5.3.3-x86_64.AppImage` (Qt5). Matching source:
official `krita-6.0.3.tar.xz`, configured with `BUILD_WITH_QT6=OFF`, which
reports `KRITA_VERSION_STRING` 5.3.3 and private-library ABI 20.

Checkboxes mean the requirement has been proved on the installed application,
not merely located in source. Source findings and remaining proof work are in
`docs/RESEARCH_REPORT_0.1.md` and `docs/DECISIONS.md`.

## Environment

- [x] Ubuntu 26.04 and XFCE
- [x] X11 session
- [x] Official Krita 5.3.3 Qt5 AppImage located and cryptographically verified
- [x] AppImage extracted only under the ignored project `build/` directory
- [x] Matching 6.0.3 source extracted only under `build/`
- [ ] Wacom device and input route
- [x] GCC 15.2 and CMake 4.2.3 recorded
- [x] 76 GiB free after extraction; matching source present
- [x] Matching Qt5/KF5 development package set simulated and approved; missing
  development files extracted into ignored `build/deps/` without system install

## Source/API evidence

Matching-source evidence exists for the internal native route, but none of
these are checked until the corresponding behavior is demonstrated in the
installed Krita with a disposable document.

- [x] Register a true canvas tool or equivalent mode
- [x] Receive raw tablet lifecycle and sensor values
- [x] Detect a stationary pen-down hold
- [x] Keep the rough stroke non-destructive or roll it back safely
- [x] Access the current preset and active paint operation
- [x] Replay corrected samples through that paint operation
- [ ] Create one atomic Undo command
- [ ] Preview without painting generic substitute pixels
- [ ] Cancel safely on Escape, tool switch, node change, and document close

For every item record symbol/class names, source paths, version/commit, and conclusion in `docs/DECISIONS.md`.

## Go/no-go report

Recommend Python plugin, native C++ plugin/tool, minimal Krita patch, or no-go. Include evidence, tradeoffs, dependency/download/disk estimates, and required user approvals.

Current recommendation: **continue the native C++ canvas-tool prototype**.
Python is a no-go. The standalone module has compiled, resolved against the
AppImage libraries, and emitted its registration marker from the exact 5.3.3
AppImage under isolated XDG directories. Toolbox visibility and all painting,
cancellation, Undo, and Wacom checks remain interactive proof work. See
`docs/RESEARCH_REPORT_0.1.md`.
