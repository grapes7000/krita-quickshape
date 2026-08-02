# AGENTS.md

These instructions apply to every file in this repository and to every coding agent working on it.

## Mission

Build one unified Krita tablet tool that cleans a captured stroke and, only when confidence is high, recognizes a basic shape. The corrected path must be painted with Krita's active brush engine and current brush preset.

## Truthfulness

- Never call this a working plugin until it is installed and interactively tested in Krita.
- Keep these states distinct: planned, implemented, compiled, unit-tested, tested in Krita, and confirmed with the user's Wacom pen.
- Never hide a failed requirement behind an easier approximation.
- Record limitations and failed experiments in `docs/DECISIONS.md`.

## First gate: no integration before proof

Before creating Krita integration code, complete `docs/RESEARCH_CHECKLIST.md` against the user's exact installed Krita version. Prove that the chosen route can capture tablet samples, detect an end hold, replay a corrected path through the active paint operation, group Undo correctly, and cancel safely.

Do not assume the Python API supports this. If native C++ or a minimal Krita patch is required, document the evidence and obtain user approval before downloading, installing, or compiling large dependencies.

## Never substitute

The final corrected stroke may not use QPainter, a generic solid pen, SVG, a vector-only outline, a fixed round brush, or a simulated preset. Krita's actual current preset must render it.

## Preserve user data

- Never test on the user's real artwork.
- Never overwrite the system Krita installation or the user's configuration.
- Do not change brushes, tags, shortcuts, workspaces, tablet mappings, resources, themes, or unrelated plugins.
- Back up any configuration file before an approved edit.
- A custom Krita build must be installed side-by-side under a user-controlled prefix.
- Provide a reversible uninstall path.

## Permission boundaries

- No `sudo`, package installation, large clone/download, long compile, publishing, GitHub push, or pull request without explicit user approval.
- Explain the exact command, purpose, disk cost, and expected duration before asking.
- Never use destructive Git commands such as `git reset --hard` or `git clean -fd`.
- Do not use subagents unless the user explicitly authorizes them.

## Engineering rules

- Read applicable parent `AGENTS.md` files before changes.
- Keep geometry and classification independent from Krita and Qt where practical.
- Use document-space coordinates; make thresholds zoom- and resolution-aware.
- Preserve endpoints, detected corners, rotation, scale, and normalized pressure history.
- Prefer confidence-based fallback to general smoothing over incorrect classification.
- Keep constants named, documented, configurable, and tested.
- Keep generated files in `build/`, never in source directories.
- Make small commits with one purpose each.
- Update tests and documentation with behavioral changes.
- Compile with warnings enabled; do not silence warnings without justification.

## Required checks before a commit

Run:

```bash
./scripts/check.sh
git diff --check
git status --short
```

Do not claim success if any check fails. Report the exact failure and next action.

## Definition of done

The project is only complete after every applicable item in `docs/TEST_PLAN.md` passes, including interactive Krita and Wacom tests. Compilation alone is not completion.
