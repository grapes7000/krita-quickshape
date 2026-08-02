# Architecture Decision Log

Do not erase rejected approaches; append dated entries.

## ADR-000: Real active-brush replay is mandatory

- Status: accepted
- Decision: corrected paths must be rendered by Krita's active brush engine and current preset.
- Rejected: QPainter, SVG/vector strokes, fixed brushes, and texture approximations.
- Reason: brush texture and tablet-driven behavior are core user requirements.

## ADR-001: Geometry core remains host-independent

- Status: accepted
- Decision: core path processing uses standard C++ types without Qt/Krita headers.
- Reason: enables fast deterministic tests and avoids coupling recognition research to an unproven integration route.

### ADR-002: Reject Python and prototype a native canvas tool

- Date: 2026-08-01
- Status: accepted for feasibility prototyping; runtime proof pending
- Context: Version 0.1 needs raw tablet capture, active-preset replay, rollback,
  preview, and one atomic Undo operation on packaged Krita 5.2.2.
- Evidence: At upstream tag `v5.2.2` (commit
  `d9ba1af793aefb07c208af292f98aefc9e8ff67a`), native tools register through
  `KoToolRegistry`; `KisTool` receives `KoPointerEvent`; and
  `KisToolFreehandHelper` snapshots resources, starts
  `FreehandStrokeStrategy`, queues `KisPaintInformation`, and exposes end and
  protected cancellation paths. The LibKis Python surface registers extensions
  and dockers but exposes none of those painting internals.
- Decision: reject the Python route. The first executable experiment will be a
  native C++ canvas tool against the exact installed version. A minimal Krita
  patch is a fallback only if a standalone plugin cannot safely use the private
  ABI.
- Consequences: integration is version-coupled and cannot start from the
  runtime package alone. Interactive success is not yet claimed.
- Follow-up: obtain headers/build environment approval, then prove capture,
  hold, active-preset replacement, cancel, and Undo in a disposable document.

### ADR-003: Do not build the native prototype on the current filesystem

- Date: 2026-08-01
- Status: accepted
- Context: only 2.6 GiB is free. The matching Ubuntu source archive alone is
  182 MiB compressed, and required Krita, Qt, and KDE development headers are
  not installed.
- Evidence: local package/file audit and Ubuntu Noble source-package metadata.
- Decision: do not download source, install dependencies, or compile Krita here.
  Require at least 15 GiB free on a user-controlled build filesystem before a
  targeted experiment; reassess upward if dry-run dependency estimates require
  it.
- Consequences: version 0.1 is currently implemented only through the read-only
  research stage. No integration code may be created while the first gate is
  incomplete.
- Follow-up: connect the Wacom device, choose build storage, run package-manager
  dry runs, and request explicit approval with exact commands, disk cost, and
  duration.

### ADR-004: Retarget Gate 1 exclusively to Krita 5.3.3 Qt5

- Date: 2026-08-02
- Status: accepted; supersedes the version and capacity assumptions in ADR-002
  and ADR-003
- Context: Gate 1 now targets the official 5.3.3 Qt5 AppImage on Ubuntu 26.04
  XFCE/X11. Krita publishes 5.3.3 and 6.0.3 from the same dual-Qt source.
- Evidence: the official 6.0.3 `CMakeLists.txt` defaults
  `BUILD_WITH_QT6=OFF`, reports version 5.3.3 in that branch, and assigns
  private library ABI 20. The AppImage contains Qt5 and `libkritaui.so.20.0.0`.
  Its SHA-256 is
  `1e3fff5da006c0d2600f98a41aa2c9a7dfa49bd931f3640616f30d762db4f743`.
  The source SHA-256 is
  `9716b8b9b58d5a7c4cc755380ae2a0ba2298086aed93c63c74f0d69edb63b032`.
  Both match KDE's Details pages and have good signatures from fingerprint
  `E9FB29E74ADEACC5E3035B8AB69EB4CF7468332F`.
- Decision: drop 5.2.2 support. Use 6.0.3 private headers configured for Qt5
  and load only into the verified 5.3.3 AppImage.
- Consequences: old untracked 5.2.2 files remain untouched. Existing 5.2.2
  research is historical evidence, not proof for the new target.
- Follow-up: establish the exact Qt5/KF5 development package set and compile a
  registration-only module before implementing paint behavior.

### ADR-005: Keep a version-neutral host boundary

- Date: 2026-08-02
- Status: accepted
- Context: the required paint and tool APIs are private and version-coupled.
- Decision: Qt/Krita types must remain inside a `krita_5_3_3` adapter. Core
  code exposes stroke lifecycle, sensor samples, active-brush transaction
  begin/append/end/cancel, and typed interruption reasons.
- Consequences: future Krita targets add adapters without changing geometry or
  recognition code. A 600 ms endpoint hold and 2 document-pixel drift limit
  are named, testable Gate 1 defaults.

### ADR-006: Standalone private-ABI module is viable for Gate 1

- Date: 2026-08-02
- Status: accepted for continued prototyping; interactive paint proof pending
- Context: the first experiment needed to show that an out-of-tree tool module
  can link and register in the official AppImage without installing into Krita.
- Evidence: the 6.0.3 source configured successfully with
  `BUILD_WITH_QT6=OFF` using development files isolated under `build/deps/`.
  `kritaquickshape_krita_5_3_3.so` compiles with private ABI 20, and `ldd -r`
  resolves it entirely against the AppImage's bundled Krita/Qt5/KF5 libraries
  without missing symbols. The exact 5.3.3 AppImage, launched with temporary
  HOME/XDG paths and `KRITA_PLUGIN_PATH`, emitted
  `QuickShape: registered KritaShape/QuickShapeTool`.
- Failed experiment: the initial output name
  `quickshape_krita_5_3_3.so` was skipped because `KoJsonTrader` scans Linux
  plugin filenames beginning with `krita`. Renaming the output fixed discovery.
- Decision: continue with the standalone `krita_5_3_3` adapter; do not start a
  side-by-side Krita build yet.
- Consequences: registration is compiled and load-tested, not yet visually
  confirmed in the toolbox. The first 600 ms cancellation/replay slice now
  compiles, but has not been exercised on a disposable canvas with mouse or pen.
  CMake reports an RPATH conflict warning because compile-time system Qt/KF
  imported targets share SONAMEs with bundled libraries; runtime resolution was
  explicitly checked against the AppImage and remains a release-hardening item.

### ADR-007: Preserve bundled plugins and trust the live freehand transaction

- Date: 2026-08-02
- Status: accepted for interactive Gate 1 testing
- Context: setting `KRITA_PLUGIN_PATH` to a directory containing only
  QuickShape disabled Krita's bundled resource and canvas plugins. In the
  5.3.3 AppImage, `KisToolFreehand::beginPrimaryAction()` can also leave the
  public tool mode at hover while its private freehand helper owns a live paint
  transaction, causing inherited motion and pen-up handlers to discard input.
- Evidence: an overlay containing all 170 bundled plugins plus QuickShape
  restores normal startup. Instrumentation then reported `mode 0` together
  with `transaction true` for every pen-down event.
- Decision: the isolated launcher constructs a complete plugin overlay and
  uses software OpenGL. The adapter treats the helper's running stroke as the
  authoritative lifecycle state and invokes the protected stroke append/end
  operations while that transaction is live. Private inline symbols are
  hidden so only Qt's two plugin entry points are exported.
- Consequences: startup no longer loses Krita resources, and pointer motion is
  delivered to the real active-preset paint operation despite the public mode
  mismatch. Interactive testing confirmed ordinary rough strokes and confirmed
  that a qualifying stationary hold cancels the rough stroke and starts the
  endpoint replacement. The zero-length Gate 1 placeholder can be visually
  blank with the active preset. Mouse/tablet attribution, sensor coverage,
  cancellation cases, and one-step Undo still require explicit confirmation.

### ADR-008: Replay the corrected geometry through the active paint operation

- Date: 2026-08-02
- Status: accepted; interactively confirmed
- Context: the first cancellation/replay proof used a zero-length endpoint
  path, which correctly removed the rough stroke but could leave no visible
  mark with the active preset.
- Decision: deduplicate and arc-length resample the captured stroke, apply
  corner-aware smoothing, remap pressure and tablet sensors by normalized arc
  length, then submit every corrected segment to the same active-preset
  freehand transaction.
- Evidence: interactive Krita 5.3.3 testing confirmed that a held rough stroke
  remains visible as a smoother replacement after replay. Automated coverage
  also verifies tangential-pressure remapping.
- Consequences: Gate 1 now demonstrates visible corrected-path replay. Exact
  line, circle, ellipse, polygon, and star snapping remains Gate 3 recognition
  work rather than part of this general smoothing fallback.

## ADR template

### ADR-NNN: Title

- Date:
- Status: proposed/accepted/rejected/superseded
- Context:
- Evidence:
- Decision:
- Consequences:
- Follow-up:
