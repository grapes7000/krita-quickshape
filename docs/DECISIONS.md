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

## ADR template

### ADR-NNN: Title

- Date:
- Status: proposed/accepted/rejected/superseded
- Context:
- Evidence:
- Decision:
- Consequences:
- Follow-up:
