# FileSoul - Pyri's Cry

FileSoul is a terminal-based C program that scans a folder, assigns each file a simple personality and mood, and helps the user decide what to do with each file.

## Build

```sh
gcc *.c -o filesoul.exe
```

The active build uses root-level `.c` and `.h` files. The `src/` folder is a preparation area and is not part of the current build.

## Run

```sh
.\filesoul.exe
```

The program asks for:

- folder path, with empty input meaning `.`
- whether guarded real deletion should be enabled
- how many files to discuss
- per-file or batch choices

## Current Features

- Windows folder scanning
- file path, name, extension, size, and modified time storage
- `FileSoul` data separated from `FileNode` linked list nodes
- extension-based file type classification
- mood, personality, dialogue, and interest score calculation
- `InterestCalculator` function pointer
- sorted dialogue flow by interest score
- choices: open, keep, delete candidate, ignore
- batch choices: ignore remaining files or mark remaining files as delete candidates
- delete candidate preview
- guarded real deletion with exact `DELETE` confirmation
- protected file checks
- extension-by-extension statistics
- report generation at `results/reports/report.txt`

## Deletion Safety

Real deletion is disabled by default. It only runs when:

1. the user enables real deletion at startup,
2. files are marked as delete candidates,
3. the delete preview is shown,
4. the user types `DELETE` exactly,
5. the file passes protection checks,
6. the file is inside the scanned folder.

Protected files include executables, scripts, repository files, source files, reports, directories, paths containing `..`, and files outside the scan root.

## Reports

The report includes scan summary, delete results, extension statistics, and per-file details. If `results/reports/report.txt` cannot be opened, the program attempts to write `report.txt`.
