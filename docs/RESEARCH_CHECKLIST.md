# Read-only Feasibility Checklist

Complete this before integration code or dependency installation.

## Environment

- [ ] Linux Mint and XFCE versions
- [ ] X11 or Wayland session
- [ ] Krita version, executable, and packaging method
- [ ] Krita resource and plugin paths
- [ ] Qt and KDE Framework versions
- [ ] Wacom device and input route
- [ ] Compiler, CMake, Ninja/Make, Git
- [ ] RAM, free disk, and existing matching source/headers
- [ ] Existing custom plugins/configuration to preserve

## Source/API evidence

- [ ] Register a true canvas tool or equivalent mode
- [ ] Receive raw tablet lifecycle and sensor values
- [ ] Detect a stationary pen-down hold
- [ ] Keep the rough stroke non-destructive or roll it back safely
- [ ] Access the current preset and active paint operation
- [ ] Replay corrected samples through that paint operation
- [ ] Create one atomic Undo command
- [ ] Preview without painting generic substitute pixels
- [ ] Cancel safely on Escape, tool switch, node change, and document close

For every item record symbol/class names, source paths, version/commit, and conclusion in `docs/DECISIONS.md`.

## Go/no-go report

Recommend Python plugin, native C++ plugin/tool, minimal Krita patch, or no-go. Include evidence, tradeoffs, dependency/download/disk estimates, and required user approvals.
