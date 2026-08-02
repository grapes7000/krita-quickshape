# Test Plan

## Geometry fixtures

- Nearly straight wobbly line
- C-curve and S-curve
- Mixed curves and intentional corners
- Zigzag with sharp vertices
- Rough circle and rotated ellipses
- Triangles at multiple rotations
- Axis-aligned and rotated rectangles
- Rough five-point star
- Irregular closed character/object silhouette
- Short, long, nearly closed, duplicate-sample, and slow-input strokes
- Multiple zoom levels and document resolutions
- Increasing, decreasing, and multi-peak pressure histories
- Ambiguous shapes that must fall back to smoothing

## Acceptance assertions

- Jitter decreases measurably without significant silhouette drift.
- Endpoints remain stable unless an accepted closed shape requires closure.
- Curves remain curves; corners remain corners.
- Irregular contours are not forced into primitives.
- Recognized shapes preserve position, scale, and rotation.
- Pressure extrema remain in corresponding normalized locations.

## Interactive Krita matrix

Test on a disposable paint layer with:

- Basic opaque ink preset
- Pressure-sensitive ink preset
- Textured/chalk preset
- Opacity-varying preset
- Eraser preset, if supported

For each preset verify immediate lift, hold correction, preview, commit, Escape, Undo, tool switch, layer switch, document close, rapid repeated strokes, and a long stroke.

## Required evidence

Record exact Krita build, session type, tablet, preset names, steps, expected result, actual result, pass/fail, and known limitation. Compilation and unit tests do not substitute for interactive tests.

## Performance

Measure capture overhead and fitting latency for short, normal, and pathological strokes. Check repeated use for memory growth and UI stalls.
