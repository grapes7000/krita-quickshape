# Proposed Architecture

The host-neutral boundary is stable; Krita-private implementation details are
confined to a versioned adapter.

## Modules

| Module | Responsibility | Krita-dependent |
|---|---|---:|
| Input adapter | Capture tablet samples and lifecycle events | Yes |
| Stroke model | Positions, timestamps, pressure, tilt, rotation, arc length | No |
| Preprocessor | Deduplication, document-space resampling, normalization | No |
| Feature extractor | Closure, curvature, corners, extrema, bounds | No |
| General fitter | Corner-aware smoothing and spline fitting | No |
| Shape fitters | Line, circle, ellipse, polygon, star candidates | No |
| Classifier | Confidence scoring and safe fallback | No |
| Sensor remapper | Reparameterize sensor history on corrected path | Mostly no |
| Preview adapter | Non-destructive corrected-path preview | Yes |
| Paint replay adapter | Replay through the active Krita paint operation | Yes |
| Transaction adapter | Commit/cancel and one-step Undo | Yes |
| Tool options | Persistent, safe user controls | Yes |

The first adapter is named `krita_5_3_3`. It targets only the official Qt5
AppImage and may include private headers from the matching 6.0.3 source tree.
Geometry, recognition, and `quickshape::HostAdapter` must not include Qt or
Krita types. Future host versions add adapters instead of conditionals in the
core.

## Dependency direction

`krita_5_3_3 adapter -> host contract -> application pipeline -> geometry core`

The geometry core must never include Krita or Qt headers. Adapters may translate Krita/Qt types into core types at the boundary.

## State machine

- `Idle`
- `Capturing`
- `EndpointHeld`
- `Fitting`
- `Previewing`
- `Committing`
- cancellation returns directly to `Idle` after the active transaction is
  rolled back

Every non-idle state must have a safe path to `Idle` on Escape, tool switch, document close, layer change, or error.

## Open integration question

Source inspection confirms the required helper route. The remaining critical
unknown is whether a standalone module built against the matching private
headers can load safely into the AppImage and preserve one-step Undo during
cancel/replay. This requires compiled and interactive proof.
