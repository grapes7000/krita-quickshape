# Version 0.1 Feasibility Report

Date: 2026-08-01

Status: read-only research implemented; native prototype not yet implemented.

## Installed target

| Item | Evidence |
|---|---|
| OS | Linux Mint 22.3 (Zena), Linux 7.0.0-28 |
| Desktop/session | XFCE on X11 |
| Krita | `/usr/bin/krita`, deb `1:5.2.2+dfsg-2build8`, not an AppImage |
| Upstream source | tag `v5.2.2`, commit `d9ba1af793aefb07c208af292f98aefc9e8ff67a` |
| UI dependencies | Qt 5.15.13, KDE Frameworks 5.115 |
| Build tools | GCC 13.3, CMake 3.28.3, Ninja 1.11.1, Make 4.3, Git 2.43 |
| Capacity | 31 GiB RAM; 2.6 GiB free on the project filesystem |
| Tablet | `xsetwacom` is installed; no device was visible during the audit |

Krita's saved system report at
`~/.local/share/krita-sysinfo.log` supplied the application, Qt, OS, desktop,
and packaging facts. Running `krita --version` without a display aborted; that
failure is not treated as an interactive Krita failure.

System resources are under `/usr/share/krita`; native modules are under
`/usr/lib/x86_64-linux-gnu/kritaplugins`; user resources and Python plugins are
under `~/.local/share/krita` and `~/.local/share/krita/pykrita`.

Existing user state to preserve includes Krita configuration under `~/.config`,
resource bundles under `~/.local/share/krita`, and the existing `BrushSfx`,
`CustomBrushCursor`, `pink_scrollbar`, and `style_sheet_loader` Python plugins.
No file in those locations was changed.

## Exact-version API findings

The source references below use KDE's official repository mirror at tag
`v5.2.2`.

### Canvas tool and tablet lifecycle

- `plugins/tools/basictools/default_tools.cc` registers native tool factories
  with `KoToolRegistry::instance()->add(...)`.
- `libs/ui/tool/kis_tool.h` exposes `beginPrimaryAction`,
  `continuePrimaryAction`, and `endPrimaryAction` with `KoPointerEvent`, plus
  high-resolution-event opt-in.
- `libs/ui/tool/kis_tool_freehand.cc` converts pointer positions into image
  pixels and forwards the lifecycle to `KisToolFreehandHelper`.
- `KisPaintingInformationBuilder::startStroke` and `continueStroke` create
  `KisPaintInformation`, whose exact-version constructor carries position,
  pressure, X/Y tilt, rotation, tangential pressure, perspective, and time.

Conclusion: a native canvas tool can receive the required lifecycle and sensor
data. A timer owned by that tool can detect a stationary pen-down hold, but the
behavior still needs an interactive Wacom test.

### Active brush replay

- `libs/ui/tool/kis_tool_freehand_helper.cpp::initPaintImpl` creates a
  `KisResourcesSnapshot` from the image, node, and canvas resource provider.
  That snapshot owns the current preset state for the stroke.
- The same method starts `FreehandStrokeStrategy` through the image's
  `KisStrokesFacade`.
- `paintAt`, `paintLine`, and `paintBezierCurve` enqueue
  `FreehandStrokeStrategy::Data` containing `KisPaintInformation`.
- `libs/ui/tool/strokes/freehand_stroke.h` defines the active-paint-operation
  job types for points, lines, and curves.
- The installed `libkritaui.so.19` exports these helper symbols, confirming that
  the examined path exists in the packaged binary.

Conclusion: corrected samples can use Krita's actual active preset and paint
operation. QPainter, SVG, and generic-pen substitution are unnecessary and
remain forbidden.

### Commit, rollback, preview, and Undo

- `KisToolFreehandHelper::endPaint` calls `endStroke` on the running image
  stroke.
- `KisToolFreehandHelper::cancelPaint` stops helper timers and calls
  `cancelStroke`, leaving no completed rough-stroke transaction.
- A derived helper can expose cancellation and its protected high-level
  `paintLine`/`paintBezierCurve` methods to a native tool.
- A candidate hold flow is: paint the rough stroke as a running operation;
  cancel it on a qualifying hold; start a replacement operation from the same
  active resources; enqueue corrected `KisPaintInformation`; end on pen-up or
  cancel on interruption. Only the replacement operation should reach Undo.
- The replacement may be shown while its stroke is still cancellable, providing
  an active-brush preview without committing substitute pixels.

Conclusion: source supports the design, but cancellation after visible rough
painting, a single Undo entry, and unchanged-layer cancellation must be proved
interactively before their checklist items can be checked.

The custom tool must override the stock freehand behavior on tool deactivation:
Krita 5.2.2's `KisToolFreehand::deactivate` ends a running stroke, while
QuickShape requires cancellation during an uncommitted correction. Node and
document lifetime signals must similarly route to cancellation before adapter
pointers become invalid.

### Python boundary

The 5.2.2 `libs/libkis/Krita.h` scripting surface registers `Extension` and
`DockWidgetFactoryBase`. `libs/libkis/Canvas.h` exposes view properties such as
zoom, rotation, mirroring, wraparound, and instant-preview mode. It does not
expose `KoToolRegistry`, raw `KoPointerEvent`, `KisToolFreehandHelper`,
`KisPaintInformation`, `KisResourcesSnapshot`, or `KisStrokesFacade`.

Conclusion: Python cannot meet canvas-tool capture and active-paint-operation
replay requirements. A Python prototype would be an invalid approximation.

## Route decision

1. **Python plugin: no-go.** Required capture, replay, and transaction APIs are
   absent from the scripting API.
2. **Native C++ tool plugin: conditional go and preferred prototype.** The
   required implementation path exists and the runtime libraries export it.
3. **Minimal Krita patch: fallback.** Use only if a standalone module cannot be
   built and loaded safely against Krita's private 5.2.2 ABI.
4. **No-go:** choose this if the native prototype fails active-preset replay,
   cancellation, or atomic Undo in the disposable interactive test.

## Build and approval boundary

The Mint package installs runtime libraries but no Krita headers, no Krita CMake
development package, and almost none of the required Qt/KDE development stack.
Ubuntu provides the matching source as a 182 MiB compressed archive. Extracted
source, build dependencies, and even a targeted build would exceed the current
2.6 GiB safety margin; a full side-by-side build commonly needs many additional
gigabytes and must not be attempted on this filesystem.

Before prototype implementation, the user must approve the exact download and
dependency-install commands after at least 15 GiB of working space is available.
The request must include measured package download/install estimates and an
expected duration generated by dry-run package-manager commands. No `sudo`,
package install, source download, or compile has been performed.

## Remaining Gate 0 work

- Connect the Wacom pen and record `xinput list` and `xsetwacom --list devices`.
- Choose or provide a filesystem with at least 15 GiB free for a side-by-side
  native build experiment.
- Run non-mutating package-manager simulations to calculate exact dependency
  costs, then obtain approval.
- Build and run the minimal disposable-document prototype.
- Record active-preset replay, hold behavior, cancellation, and one-step Undo
  results before checking any source/API requirement.

## Sources

- <https://github.com/KDE/krita/tree/v5.2.2>
- <https://github.com/KDE/krita/blob/v5.2.2/libs/ui/tool/kis_tool.h>
- <https://github.com/KDE/krita/blob/v5.2.2/libs/ui/tool/kis_tool_freehand.cc>
- <https://github.com/KDE/krita/blob/v5.2.2/libs/ui/tool/kis_tool_freehand_helper.h>
- <https://github.com/KDE/krita/blob/v5.2.2/libs/ui/tool/kis_tool_freehand_helper.cpp>
- <https://github.com/KDE/krita/blob/v5.2.2/libs/ui/tool/strokes/freehand_stroke.h>
- <https://github.com/KDE/krita/blob/v5.2.2/plugins/tools/basictools/default_tools.cc>
- <https://github.com/KDE/krita/blob/v5.2.2/libs/libkis/Krita.h>
- <https://github.com/KDE/krita/blob/v5.2.2/libs/libkis/Canvas.h>
- <https://launchpad.net/ubuntu/noble/+source/krita/1%3A5.2.2+dfsg-2build8>
