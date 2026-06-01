# FileSoul Implementation Plan

## Completed in Current Core Flow

- Keep the active build on root-level `.c/.h` files.
- Split file data into `FileSoul` and linked list nodes into `FileNode`.
- Add Windows folder scanning with `FindFirstFileA` / `FindNextFileA`.
- Add file type, mood, personality, dialogue, and interest score assignment.
- Use a function pointer for interest score calculation.
- Add terminal choices: open, keep, delete candidate, ignore.
- Keep delete behavior as delete-candidate marking only.
- Add summary statistics.
- Write report output to `results/reports/report.txt`.

## Next Step 1: Improve Scanner Quality

- Add better path length handling.
- Consider recursive scanning only if the project scope allows it.
- Add clearer messages for inaccessible folders.

## Next Step 2: Improve User Interaction

- Add a way to skip all remaining files.
- Add compact display mode for folders with many files.
- Consider not showing build artifacts by default.

## Next Step 3: Improve Statistics

- Implement extension-by-extension counts and total sizes.
- Format byte sizes as KB/MB/GB.

## Next Step 4: Clean Build Artifacts

- Decide as a team whether `filesoul.exe` should remain tracked.
- Do not run `git rm --cached filesoul.exe` until approved.

## Still Out of Scope

- Real file deletion
- GUI
- Background service
- Windows overlay
- Moving the root build into `src/`
