# FileSoul - Pyri's Cry

FileSoul is a terminal-based C project that gives files simple personalities and dialogue so a user can decide how to organize them.

The current GitHub version uses root-level C files. The repository also contains `src/` preparation files copied from the user's local development workspace for later refactoring, but those files are not part of the current build yet.

## Environment

- Windows
- GCC or MinGW GCC

## Build

Run this from the GitHub-connected repository root:

```sh
gcc *.c -o filesoul.exe
```

## Run

```sh
.\filesoul.exe
```

## Current Repository Structure

The current build uses these root-level files:

- `main.c`: creates sample file nodes and controls the current demo flow
- `file_node.c/.h`: file data and singly linked list management
- `personality.c/.h`: file type classification, personality assignment, and dialogue generation
- `dialogue_view.c/.h`: dialogue output
- `report.c/.h`: report writing function, not yet called by `main.c`

The `src/` folder is a staging area for selected local development modules from `C:\Users\LG\C_Programing\src`. It currently exists to prepare a later cleanup, not to replace the root-level team code.

## Current Features

- `FileNode` singly linked list
- Extension-based file type classification
- Size/type-based personality assignment
- Dialogue generation and output
- Basic report writer function

## Safety Notes

- Do not implement real file deletion.
- Delete-related choices should only mark files as delete candidates.
- `filesoul.exe` is currently tracked, but it is a build artifact. Routine changes to it should not be committed.

## Future Direction

- Separate file data into `FileSoul` and list nodes into `FileNode`.
- Add real folder scanning.
- Add user choices: open, keep, delete candidate, ignore.
- Write reports to `results/reports/report.txt`.
- Move toward a modular `src/` build only after the team agrees.
