# QuickShape Behavioral Specification

Reverse-engineered from Procreate's QuickShape feature, annotated with our
implementation status and Krita-specific adaptations.

## 1. Trigger mechanism

### Procreate behavior

The user draws a stroke and holds the stylus stationary at the endpoint. After
a configurable delay (0.10–1.50s, default ~0.75s), the stroke snaps to a
recognized shape. No visual cue during the hold — the freehand stroke stays
visible until the snap fires.

Alternative triggers: gesture reassignment and Apple Pencil Pro squeeze invoke
QuickShape on the last stroke drawn.

### Our implementation

600ms hold with 2-document-pixel drift limit. QTimer-based detection in the
Krita adapter (ADR-005).

### Gap

- Hold delay needs to be user-configurable (100–1500ms range) via Tool Options.
- No alternative trigger needed — Krita lacks Procreate's gesture system.

---

## 2. Shape recognition

### Procreate behavior

Recognized shapes: **line, arc, polyline, ellipse (including circle), triangle,
quadrilateral (including square/rectangle)**.

NOT recognized: stars, pentagons, hexagons, spirals, S-curves. These either
become polylines or don't snap at all.

Ambiguity handling: single best-guess — no candidate picker. Curves that aren't
cleanly elliptical fall back to straight-line polyline segments.

### Our implementation

Recognized: line, arc, circle, ellipse, triangle, rectangle, general polygon,
five-point star. Confidence-based classifier with `ShapeType::None` fallback
that applies general corner-aware smoothing.

### Gap

- Arc fitting implemented (circular arc for open strokes). Elliptical arc
  fitting could be added later but circular arcs cover the most common case.
- Our smoothing fallback when no shape matches is BETTER than Procreate, which
  has no smoothing fallback at all.
- Stars are a bonus shape Procreate doesn't have.

---

## 3. Snap behavior

### Procreate behavior

Instantaneous replacement — no morphing animation. The freehand stroke
disappears and the corrected shape appears in a single frame.

### Our implementation

Cancel rough stroke (`cancelPaint`) then start replacement stroke. Should be
perceptually instantaneous.

### Gap

- Verify no visible flicker between cancel and replacement render.

---

## 4. Post-snap interaction

### Procreate behavior (SIGNATURE FEATURE)

While the user keeps the stylus down after the snap:

1. **Drag** to scale and rotate the shape in real-time.
2. **Second finger on screen** converts to perfect form:
   - Ellipse → circle
   - Rectangle → square
   - Triangle → equilateral
3. **Second finger + drag** rotates in 15° increments.
4. **Drag** repositions the shape.
5. **Lift stylus** commits the final shape to the canvas.

### Our implementation

Not implemented. The corrected stroke is committed immediately.

### Gap

This is the biggest missing piece. Implementation options in Krita:

- **Option A**: keep the replacement stroke uncommitted (`endPaint` deferred
  until pen-up). On drag, cancel and re-replay with transformed geometry.
  Expensive but uses the real brush engine.
- **Option B**: use a QPainter overlay for the drag preview, then replay the
  final transform through the brush engine on commit. Faster but the preview
  won't match the brush texture exactly.
- **Modifier key mapping**: Shift = perfect form, Ctrl+drag = 15° rotation
  snap (matching Krita conventions).

---

## 5. Edit Shape mode

### Procreate behavior

After lifting the stylus, an "Edit Shape" button appears briefly. Tapping it
places blue control-point nodes on the shape's vertices/curves. The user can
drag nodes to adjust the shape. Disappears if the user draws anything else.

### Our implementation

Not implemented.

### Gap

Lower priority — defer to Gate 6 or later. Would require retaining shape
metadata after commit and providing a node-editing overlay. Krita already has
vector shape tools that partially cover this need.

---

## 6. Pressure and brush handling

### Procreate behavior

The corrected shape is **re-rendered** through the active brush along the new
geometric path. It is NOT a pixel transformation of the original stroke.

Pressure is approximately preserved via remapping to the new path. Brush
texture is re-applied fresh — the grain pattern differs from the original.
Taper (start/end thickness) is preserved but can look uneven on corrected
shapes when path length changes significantly.

Procreate warns: "Don't use a brush with a lot of taper or pressure sensitivity
with QuickShape, as the beginning of your shape may be thinner or thicker than
the end."

### Our implementation

`remap_sensors()` in `core/src/stroke.cpp` does normalized arc-length remapping
of pressure, tilt, rotation, and tangential pressure. Replay uses the active
Krita brush preset via `KisToolFreehandHelper`.

### Gap

Mostly implemented. The approach matches Procreate's. Verify that taper and
pressure artifacts are acceptable, not glitchy.

---

## 7. Undo behavior

### Procreate behavior

The original stroke + correction = one undo step. Undo removes the corrected
shape entirely — cannot revert to the rough freehand version.

During the hold (before lifting), undo cancels the QuickShape operation.

### Our implementation

Rough stroke is cancelled (`cancelPaint` — removed from undo history).
Replacement stroke is a new `FreehandStrokeStrategy`. Undo should remove only
the replacement.

### Gap

- Verify cancelled rough stroke does NOT appear in undo history.
- Verify undo during the hold phase cancels correctly.

---

## 8. Smoothing vs. shape snapping

### Procreate behavior

QuickShape and StreamLine are completely separate:

- **StreamLine**: real-time smoothing during drawing (per-brush setting). Removes
  hand tremor as you draw.
- **QuickShape**: geometric snapping after drawing on hold. Either snaps to a
  known shape or doesn't activate.

No smoothing fallback in QuickShape. If the stroke doesn't match a shape, it
stays as freehand.

### Our implementation

Our tool combines both:

- If a shape is recognized with sufficient confidence, snap to it.
- If no shape matches (`ShapeType::None`), apply general corner-aware smoothing.

### Gap

This is a **feature advantage** over Procreate. Every held stroke gets improved,
not just geometric ones. Keep this behavior. Document it as "intelligent
smoothing with optional shape snapping."

---

## 9. Visual feedback

### Procreate behavior

- **During hold**: no visual cue (no progress bar, countdown, or cursor change).
- **At snap**: sudden replacement — no animation.
- **Post-snap drag**: shape updates in real-time as user drags.
- **Edit Shape mode**: blue dots at control points.

### Our implementation

No visual cue during hold.

### Gap

Matches Procreate's no-cue approach. Could optionally add a status-bar message
("QuickShape activated") but this is not required.

---

## 10. Settings

### Procreate settings

Minimal:
- Hold delay slider (0.10–1.50s)
- Trigger gesture assignment

NO per-shape toggles, confidence threshold, or sensitivity control.

### Our planned settings

- Hold duration (100–1500ms, default 600ms)
- Correction strength
- Shape recognition toggle and confidence threshold
- Close-shape distance, corner-preservation strength
- Preview toggle, correct-every-stroke toggle
- Bypass modifier key
- Reset to defaults

### Gap

We offer more configurability. Ensure defaults are tuned so the tool works well
out of the box without touching settings, matching Procreate's zero-config
philosophy.

---

## Implementation priority

| Priority | Feature | Procreate has it | We have it | Difficulty |
|----------|---------|:---:|:---:|:---:|
| 1 | Configurable hold delay | Yes | Partial | Easy |
| 2 | Arc fitting | Yes | Yes | Done |
| 3 | Post-snap drag interaction | Yes | No | Hard |
| 4 | Perfect-form modifier (Shift) | Yes | No | Medium |
| 5 | 15° rotation snapping | Yes | No | Easy |
| 6 | Edit Shape mode | Yes | No | Hard |
| 7 | Tool Options panel | No | Planned | Medium |
| — | Smoothing fallback | No | Yes | Done |
| — | Star recognition | No | Yes | Done |
| — | General polygon recognition | Partial | Yes | Done |

---

## Krita-specific adaptations

- **No multi-touch**: Krita on desktop doesn't reliably support simultaneous
  pen + finger. Procreate's "second finger" features map to modifier keys:
  Shift for perfect form, Ctrl for constrained rotation.
- **Brush engine differences**: Krita's brush engine is more configurable than
  Procreate's. Pressure remapping may need tuning per preset.
- **Tool registration**: our tool is a `KoToolRegistry` canvas tool, not a
  modifier on the existing freehand tool. Users select it explicitly.
- **StreamLine equivalent**: Krita has built-in brush smoothing (stabilizer).
  Our QuickShape tool's smoothing fallback is separate and complementary.
