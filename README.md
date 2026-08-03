# Krita QuickShape

An early-stage, testable project scaffold for a unified Krita drawing tool that cleans wobbly pen strokes, preserves intentional curves and corners, and optionally recognizes geometric shapes while using Krita's currently selected brush preset.

> Status: semi-working Krita tool, actively debugging and optimizing. The sole
> target is the official Krita 5.3.3 Qt5 AppImage, whose matching source is the
> Krita 6.0.3 dual-Qt source archive built with its default Qt5 mode. The tool
> loads into Krita and performs basic stroke correction; shape recognition
> integration is in progress.

## Intended experience

1. Select any Krita brush preset.
2. Draw normally with a tablet pen.
3. Lift immediately to keep ordinary freehand output.
4. Hold briefly at the end to preview and commit a cleaned stroke.
5. Clear shapes may snap to a line, curve, circle, ellipse, triangle, rectangle, polygon, or five-point star; uncertain strokes receive general smoothing instead.

The tool must remain a single drawing mode. It must not require switching among Krita's separate shape tools.

## Non-negotiable requirement

Corrected strokes must be replayed through Krita's real active brush engine and current preset. Generic Qt painting, SVG/vector substitution, or fixed-brush approximations are not acceptable.

## Repository map

- `AGENTS.md` — permanent rules for coding agents
- `docs/AI_PROJECT_BRIEF.md` — complete implementation contract
- `docs/ARCHITECTURE.md` — proposed subsystem boundaries
- `docs/ROADMAP.md` — gated development stages
- `docs/TEST_PLAN.md` — automated and interactive acceptance testing
- `docs/USER_GIT_WORKFLOW.md` — simple Git checklist for the project owner
- `docs/DECISIONS.md` — architecture decision log
- `docs/RESEARCH_CHECKLIST.md` — read-only feasibility audit
- `docs/RESEARCH_REPORT_0.1.md` — exact-version findings and 0.1 build gate
- `core/` — dependency-free geometry model and initial smoothing API
- `tests/` — buildable smoke tests

## Build the current scaffold

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Or run:

```bash
./scripts/check.sh
```

## Current scope

The geometry core implements deduplication, arc-length resampling, corner detection, corner-aware smoothing, pressure/sensor remapping by normalized arc length, and a version-neutral stroke/transaction lifecycle contract. The fitting module recognizes lines, circles, ellipses, triangles, rectangles, general polygons, and five-point stars with confidence scoring and safe fallback for ambiguous strokes. All algorithms are host-independent (no Qt/Krita dependency) and covered by unit tests.

The `krita_5_3_3` adapter loads into the official Krita 5.3.3 AppImage and performs stroke capture, hold detection, cancellation, and corrected-path replay through the active brush preset. Shape recognition (Gate 3/4) is implemented in the core and being integrated into the live tool. Currently debugging and optimizing the correction flow.

## License

GPL-3.0-or-later. See `LICENSE`.
