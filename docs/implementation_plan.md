# FileSoul Implementation Plan

## Completed Core Work

- Root `.c/.h` build kept active.
- `FileSoul` and `FileNode` separated.
- Windows folder scanner added.
- Guarded real deletion added.
- Delete preview and exact `DELETE` confirmation added.
- Protected file checks added.
- Delete results stored and reported.
- Batch dialogue choices added.
- Dialogue file count limit added.
- Extension statistics added.
- Report content strengthened.

## Next Step 1: Demo Stability

- Add clearer prompts for large folders.
- Add a default sample/demo mode.
- Consider hiding build artifacts and repository files from normal scans.

## Next Step 2: Scanner Improvements

- Normalize paths more consistently.
- Improve long path handling.
- Add optional recursive scanning if project scope allows it.

## Next Step 3: Statistics Improvements

- Sort extension statistics by size or count.
- Add clearer Korean labels for all report fields.
- Improve byte formatting inside reports.

## Next Step 4: Build Policy

- Decide whether `filesoul.exe` should remain tracked.
- Do not run `git rm --cached filesoul.exe` until explicitly approved.

## Out of Scope

- GUI
- Background service
- Windows overlay
- Moving the root build into `src/`
- Trash/recycle-bin integration
