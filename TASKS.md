# FileSoul TASKS.md

## 1. Project Goal

FileSoul is a terminal-based file management program written in C.

The goal is to help users organize files in a folder by assigning each file a personality and mood, then presenting a dialogue-style interface for user decisions.

The final program should support:

- Folder scanning
- File name, path, extension, size, and modified time storage
- File type classification
- Personality assignment
- Mood assignment
- Interest score calculation
- Dialogue generation
- User choices
- Delete candidate marking
- Guarded real deletion
- Statistics output
- Report generation

## 2. Current Structure

The active build uses root-level `.c` and `.h` files.

Active files:

- `main.c`
- `file_node.c`
- `file_node.h`
- `scanner.c`
- `scanner.h`
- `personality.c`
- `personality.h`
- `dialogue_view.c`
- `dialogue_view.h`
- `stats.c`
- `stats.h`
- `report.c`
- `report.h`

Build command:

```sh
gcc *.c -o filesoul.exe
```

The `src/` folder is a preparation area for future modular cleanup. Current implementation work stays in root-level files.

## 3. Current Completed State

Implemented:

- `FileSoul` and `FileNode` role separation
- Windows folder scanning
- Extension-based file type classification
- Personality assignment
- Mood assignment
- Interest score calculation
- `InterestCalculator` function pointer
- Terminal dialogue output
- User choice handling
- Delete candidate marking
- Basic statistics output
- `results/reports/report.txt` generation
- Root `.c/.h` build

Still needed:

- Guarded real deletion flow
- Protected file checks
- Delete result recording
- Stronger extension statistics
- Dialogue limits for large folders
- Batch selection
- Better reports
- Stronger scan filters
- Size formatting
- Modified time formatting
- Demo stability

## 4. Real Deletion Policy

Real file deletion is allowed only through a guarded flow:

1. The user marks files as delete candidates.
2. The program shows the delete candidate preview.
3. Real deletion must be explicitly enabled.
4. The user must type the exact final confirmation text.
5. Protected files are checked.
6. The file must be inside the scanned folder.
7. The target must be a regular file.
8. The program calls `remove()`.
9. Success or failure is stored.
10. Delete results are written to the report.

Final confirmation text:

```text
DELETE
```

If the user does not type `DELETE` exactly, real deletion is skipped.

## 5. Protected Delete Targets

These must never be deleted, even if marked as candidates:

- `filesoul.exe`
- `*.exe`
- `*.dll`
- `*.sys`
- `*.bat`
- `*.cmd`
- `*.ps1`
- `.git` internal files
- `.gitignore`
- `AGENTS.md`
- `README.md`
- `TASKS.md`
- `working_log.md`
- `report.txt`
- `results/reports/report.txt`
- Project source files
- The running program
- Directories
- Empty paths
- Files outside the scanned root
- Paths containing `..`

## 6. P0 Required Work

- Create `TASKS.md`
- Document real deletion policy
- Update `AGENTS.md` deletion rules
- Add `delete_actions.c/h`
- Add delete candidate preview
- Add exact `DELETE` final confirmation
- Add protected file checks
- Store delete results
- Include delete results in reports
- Keep build passing
- Test real deletion only in a temporary test folder

## 7. P1 Important Work

- Extension-by-extension statistics
- Stronger report content
- Batch dialogue choices
- Dialogue file count limit
- Stronger scan filters
- README/docs updates

## 8. P2 If Time Allows

- Human-readable size formatting
- Modified time formatting
- More precise interest scoring
- EOF input stability
- `-Wall -Wextra` warning cleanup

## 9. P3 Later Work

- Recursive scanning
- Configuration file
- `src/` build conversion
- GUI
- Background service
- Trash/recycle-bin movement
- Restore feature

## 10. Completion Criteria

The current stage is complete when:

- `TASKS.md` exists
- `gcc *.c -o filesoul.exe` succeeds
- The program runs
- Folder scanning works
- User choices work
- Delete candidates work
- Real deletion only works when explicitly enabled
- Real deletion requires `DELETE`
- Protected files are not deleted
- Delete results appear in the report
- Statistics are printed
- Extension statistics are printed
- A report is generated
- README, AGENTS, docs, working_log, and TASKS match the implementation
- `filesoul.exe` changes are not committed
- The work is merged into `main` and pushed to `origin/main`
