# Proposed Architecture

This architecture is provisional until the Krita feasibility audit identifies supported extension points.

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

## Dependency direction

`Krita adapter -> application pipeline -> geometry core`

The geometry core must never include Krita or Qt headers. Adapters may translate Krita/Qt types into core types at the boundary.

## State machine

- `Idle`
- `Capturing`
- `EndpointHeld`
- `Fitting`
- `Previewing`
- `Committing`
- `Cancelled`

Every non-idle state must have a safe path to `Idle` on Escape, tool switch, document close, layer change, or error.

## Open integration question

The critical unknown is a supported way to feed corrected tablet samples into the active Krita paint operation without permanently committing the rough stroke first. Resolve this before locking the implementation route.
