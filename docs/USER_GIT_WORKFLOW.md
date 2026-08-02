# Brooke's Git Workflow Checklist

Use this checklist in the project directory. Ask an agent if any command reports an error; do not keep going blindly.

## Before work

- [ ] Enter the project: `cd ~/Projects/krita-quickshape`
- [ ] Confirm location: `pwd`
- [ ] Check current branch and changes: `git status`
- [ ] See branches: `git branch --all`
- [ ] If the project has a remote, download its newest history: `git fetch --all --prune`
- [ ] Update the current branch only when it is safe and clean: `git pull --ff-only`
- [ ] Create a focused branch: `git switch -c feature/short-description`

## While working

- [ ] Check changes often: `git status --short`
- [ ] Review the actual edits: `git diff`
- [ ] Run all checks: `./scripts/check.sh`
- [ ] Check whitespace mistakes: `git diff --check`

## Save a commit

- [ ] Stage only intended files: `git add path/to/file`
- [ ] Review staged changes: `git diff --staged`
- [ ] Commit clearly: `git commit -m "type: short description"`
- [ ] Confirm the commit: `git show --stat --oneline HEAD`

Suggested types: `docs`, `test`, `feat`, `fix`, `refactor`, `build`, and `chore`.

## Push safely

- [ ] Confirm the correct branch: `git branch --show-current`
- [ ] Confirm working state: `git status`
- [ ] Run checks again: `./scripts/check.sh`
- [ ] First push of a new branch: `git push -u origin HEAD`
- [ ] Later pushes: `git push`

Never force-push unless you understand exactly why it is necessary and explicitly approve it.

## After merging elsewhere

- [ ] Return to main: `git switch main`
- [ ] Update safely: `git pull --ff-only`
- [ ] Confirm tests: `./scripts/check.sh`
- [ ] Delete the local feature branch only after confirming it was merged: `git branch -d feature/short-description`

## Quick status bundle

When asking an agent for help, paste the output of:

```bash
pwd
git status
git branch --show-current
git log -5 --oneline --decorate
./scripts/check.sh
```
