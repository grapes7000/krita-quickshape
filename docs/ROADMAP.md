# Roadmap

## Gate 0 — Environment and API research

- Record exact OS, session, Krita packaging/version, Qt/KDE versions, tablet path, tools, RAM, and disk.
- Locate matching official source/API documentation.
- Identify supported capture, paint replay, preview, transaction, and tool registration points.
- Produce a written go/no-go decision. No large installs or builds without approval.

## Gate 1 — Feasibility prototype

- Capture a tablet stroke and sensor history.
- Detect end hold without breaking taps or normal strokes.
- Correct a trivial path.
- Replay it through the real active brush preset.
- Demonstrate cancellation and one-step Undo.
- Test in a disposable Krita document.

## Gate 2 — Geometry core

- Arc-length resampling and deduplication.
- Corner-aware general smoothing.
- Pressure/sensor remapping.
- Unit tests and benchmarks.

## Gate 3 — Primitive recognition

- Line and open curve.
- Circle and ellipse.
- Triangle, square, rectangle, and general polygon.
- Confidence scoring with safe fallback.

## Gate 4 — Five-point stars

- Detect outer and inner radial extrema.
- Preserve rotation and proportions.
- Reject ambiguous or non-five-point contours.

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
