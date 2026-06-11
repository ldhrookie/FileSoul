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
- `filesoul.exe` removed from Git tracking and kept in `.gitignore`.
- Extension statistics sorted by total size, file count, and extension name.
- Protected delete checks now write detailed block reasons.
- Dialogue output shortened for final demo stability.
- Floating message actions and optional LLM personality dialogue added.
- Monitor overlay timer now recommends one additional file after each expiry.

## Next Step 1: Demo Stability

- Keep prompts short for large folders.
- Add a default sample/demo mode.
- Keep build artifacts and repository files hidden from normal scans where possible.

## Next Step 2: Scanner Improvements

- Normalize paths more consistently.
- Improve long path handling.
- Add optional recursive scanning if project scope allows it.

## Next Step 3: Statistics Improvements

- Keep extension statistics sorted by size, then count, then extension name.
- Add clearer Korean labels for all report fields.
- Improve byte formatting inside reports.

## Next Step 4: Build Policy

- Continue using root-level `.c/.h` files for the active build.
- Keep `filesoul.exe` and generated outputs ignored.
- Leave `src/` as a preparation area until a future explicit migration.

## Out of Scope

- GUI
- Background service
- Moving the root build into `src/`
- Trash/recycle-bin integration
