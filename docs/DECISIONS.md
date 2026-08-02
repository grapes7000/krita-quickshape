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

## ADR template

### ADR-NNN: Title

- Date:
- Status: proposed/accepted/rejected/superseded
- Context:
- Evidence:
- Decision:
- Consequences:
- Follow-up:
