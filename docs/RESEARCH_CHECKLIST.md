# Read-only Feasibility Checklist

Complete this before integration code or dependency installation.

Audit target: Linux Mint's packaged Krita 5.2.2
(`1:5.2.2+dfsg-2build8`), upstream tag `v5.2.2`, commit
`d9ba1af793aefb07c208af292f98aefc9e8ff67a`.

Checkboxes mean the requirement has been proved on the installed application,
not merely located in source. Source findings and remaining proof work are in
`docs/RESEARCH_REPORT_0.1.md` and `docs/DECISIONS.md`.

## Environment

- [x] Linux Mint 22.3 and XFCE
- [x] X11 session
- [x] Krita 5.2.2, `/usr/bin/krita`, Ubuntu/Mint deb package
- [x] System and user resource/plugin paths recorded
- [x] Qt 5.15.13 and KDE Frameworks 5.115
- [ ] Wacom device and input route
- [x] GCC 13.3, CMake 3.28.3, Ninja 1.11.1, Make 4.3, Git 2.43
- [x] 31 GiB RAM, 2.6 GiB free disk; matching headers/source absent
- [x] Existing custom plugins and Krita configuration locations recorded

## Source/API evidence

Exact-version source evidence exists for the internal native route, but none of
these are checked until the corresponding behavior is demonstrated in the
installed Krita with a disposable document.

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

Current recommendation: **conditional go for a native C++ canvas-tool
prototype**. Python is a no-go. A minimal Krita patch remains the fallback if a
separately built plugin cannot link safely to the packaged internal ABI. See
`docs/RESEARCH_REPORT_0.1.md`. Building is blocked pending explicit approval,
more free disk, and a connected Wacom device for the final input-route proof.
