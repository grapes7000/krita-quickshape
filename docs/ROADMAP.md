# Roadmap

## Gate 0 — Environment and API research

Status: in progress. The audit selected a conditional native C++ route,
retargeted it to Krita 5.3.3 Qt5, and rejected Python. Runtime proof remains blocked; see
`docs/RESEARCH_REPORT_0.1.md`.

- Record exact OS, session, Krita packaging/version, Qt/KDE versions, tablet path, tools, RAM, and disk.
- Locate matching official source/API documentation.
- Identify supported capture, paint replay, preview, transaction, and tool registration points.
- Produce a written go/no-go decision. No large installs or builds without approval.

## Gate 1 — Feasibility prototype

Target: official `krita-5.3.3-x86_64.AppImage`; matching private headers from
the 6.0.3 source archive configured in Qt5 mode.

- Capture a tablet stroke and sensor history.
- Detect end hold without breaking taps or normal strokes.
- Correct a trivial path.
- Replay it through the real active brush preset.
- Demonstrate cancellation and one-step Undo.
- Test in a disposable Krita document.

## Gate 2 — Geometry core

Status: core algorithms implemented and tested. Benchmarks deferred.

- [x] Arc-length resampling and deduplication.
- [x] Corner-aware general smoothing.
- [x] Pressure/sensor remapping.
- [x] Unit tests.
- [ ] Benchmarks (short, ordinary, and long strokes).

## Gate 3 — Primitive recognition

Status: implemented and tested. Star recognition deferred to Gate 4.

- [x] Line fitting (PCA-based).
- [x] Circle fitting (Kåsa algebraic).
- [x] Ellipse fitting (covariance-axis estimation).
- [x] Triangle, rectangle, and general polygon (RDP simplification).
- [x] Confidence scoring with safe fallback to None.
- [ ] Open smooth curve fitting (spline-based, not yet implemented).

## Gate 4 — Five-point stars

Status: implemented and tested.

- [x] Detect outer and inner radial extrema.
- [x] Preserve rotation and proportions.
- [x] Reject ambiguous or non-five-point contours.
- [x] Dual detection path: direct extrema analysis and polygon-vertex pattern matching.

## Gate 5 — Product integration

- Single canvas tool and Tool Options.
- Preview/adjustment behavior.
- Persistent settings and bypass modifier.
- Safe unsupported-layer and cancellation handling.

## Gate 6 — Packaging

- Supported-version matrix.
- Side-by-side install if a custom Krita build is required.
- Install/uninstall tooling.
- Human user guide and release checklist.

No gate is complete until its tests and documentation are complete.
