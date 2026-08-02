# AI Project Brief

## Goal

Create a Procreate-style QuickShape and intelligent stroke-cleanup tool for desktop Krita on Linux Mint XFCE. It is one canvas tool, not a toolbar of separate shape commands.

## Interaction contract

1. The user selects any ordinary Krita brush preset.
2. A continuous tablet stroke records position, timestamp, pressure, tilt, rotation, and other available sensor values.
3. Immediate lift preserves ordinary freehand behavior by default.
4. Holding at the endpoint for a configurable 500–700 ms triggers correction.
5. A preview appears if supported safely; pen-up commits it.
6. Cancel leaves the layer unchanged.
7. One Undo removes the final operation without revealing an intermediate rough stroke.

## General cleanup

Remove hand jitter while preserving the intended silhouette, endpoints, curves, sharp corners, direction changes, and meaningful detail. Short strokes are the normal use case, but long continuous contours must remain safe and responsive. This is post-capture path fitting, not Krita's existing Stabilizer.

## Shape recognition

Initial target shapes: line, open smooth curve, circle, ellipse, triangle, square, rectangle, general polygon, and five-point star. Preserve position, scale, rotation, and star concavity. Use normalized fit errors and confidence thresholds. Ambiguity must fall back to general cleanup.

## Brush requirement

The corrected path must be replayed through Krita's active brush engine using the current preset, foreground color, size, opacity, flow, spacing, blend mode, texture, and replayable sensors. Remap pressure and sensor samples by normalized arc length. Generic rendering is forbidden.

## Candidate pipeline

1. Capture timestamped tablet samples.
2. Transform them into stable document coordinates.
3. Deduplicate and resample by arc length.
4. Detect endpoints, closure, curvature, and intentional corners.
5. Compute general smoothed/spline candidates.
6. Fit eligible geometric primitives separately.
7. Score normalized residuals plus topology penalties.
8. Select a primitive only above a conservative confidence threshold.
9. Remap sensor history to the selected path.
10. Preview and replay through the real Krita paint operation as one undoable transaction.

Candidate techniques include adaptive low-pass filtering, Ramer–Douglas–Peucker simplification, corner-aware cubic Bézier fitting, robust circle/ellipse fitting, polygon vertex fitting, and radial-extrema analysis for stars. No single smoother should be applied blindly to every path.

## Required Tool Options

- Hold duration
- Correction strength
- Shape recognition toggle
- Recognition confidence
- Close-shape distance
- Corner-preservation strength
- Live preview toggle
- Correct-every-stroke toggle
- Temporary bypass modifier
- Reset to safe defaults

Defaults must prioritize preservation over aggressive correction.

## Unsupported-layer behavior

Normal paint layers are required. Unsupported nodes must fail safely with a clear, non-destructive message. Document closing, tool switching, Escape, and canvas cancellation must not leave partial strokes, stale transactions, worker jobs, or invalid pointers.

## Performance

Keep input feedback responsive. Defer expensive fitting until hold detection or completion, use cancellable background computation when appropriate, avoid unsafe cross-thread Krita objects, cap pathological sample counts safely, and benchmark short, ordinary, and long strokes. Normal use must be offline.

## Delivery requirements

Supply source, automated tests, interactive test procedure, installation and uninstall instructions, architecture and decision records, version compatibility statement, known limitations, and packaging appropriate to the user's Krita installation method.
