# FileSoul - Pyri's Cry

FileSoul is a terminal-based C program that scans a folder, gives files simple personalities and moods, and asks the user how each file should be handled.

The program never deletes files. Choosing delete only marks a file as a delete candidate.

## Environment

- Windows
- GCC or MinGW GCC

## Build

Run this from the repository root:

```sh
gcc *.c -o filesoul.exe
```

The active build uses root-level `.c` and `.h` files. The `src/` folder is still a preparation area and is not part of the active build.

## Run

```sh
.\filesoul.exe
```

The program asks for a folder path. Empty input scans the current folder.

## Current Features

- Scans regular files in one folder on Windows
- Stores file path, name, extension, size, and modified time
- Separates `FileSoul` file data from `FileNode` linked list nodes
- Classifies files by extension
- Assigns mood, personality, dialogue, and interest score
- Uses a function pointer for interest score calculation
- Sorts files by interest score
- Provides terminal choices:
  - Open
  - Keep
  - Mark as delete candidate
  - Ignore
- Prints summary statistics
- Writes a report to `results/reports/report.txt`

## Main Modules

- `main.c`: terminal workflow
- `file_node.c/.h`: `FileSoul`, `FileNode`, linked list helpers
- `scanner.c/.h`: folder scanning
- `personality.c/.h`: type, mood, personality, dialogue, interest score
- `dialogue_view.c/.h`: terminal choice flow
- `stats.c/.h`: summary statistics
- `report.c/.h`: report generation

## Safety Notes

- Real file deletion is not implemented.
- Delete choices only set the delete candidate flag.
- `filesoul.exe` is currently tracked in Git, but routine build changes to it should not be committed.
