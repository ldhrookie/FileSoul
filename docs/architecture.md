# FileSoul Architecture Notes

## Current Flow

1. `main.c` asks the user for a folder path.
2. Empty input falls back to the current folder.
3. `scanner.c` scans regular files in that folder.
4. Each file is stored as `FileSoul` inside a `FileNode`.
5. `personality.c` assigns file type, mood, personality, dialogue, and interest score.
6. The list is sorted by interest score.
7. `dialogue_view.c` asks for a user choice for each file.
8. `stats.c` prints summary statistics.
9. `report.c` writes `results/reports/report.txt`.
10. The linked list is freed before exit.

## Data Structures

`FileSoul` stores file data:

- path
- name
- extension
- size
- modified time
- file type
- mood
- personality
- interest score
- dialogue
- user choice
- delete candidate flag
- extra union data

`FileNode` is now only the singly linked list node:

- `FileSoul data`
- `struct FileNode* next`

## Function Pointer

Interest score calculation uses this function pointer type:

```c
typedef double (*InterestCalculator)(const FileSoul* file);
```

`assignPersonality` receives an `InterestCalculator`, and `assignPersonalities` currently passes `calculateBasicInterest`.

## Build Structure

The active build still uses root-level C files:

```sh
gcc *.c -o filesoul.exe
```

The repository `src/` folder remains a preparation area and is not compiled in this step.

## Safety

Delete behavior is limited to marking files as delete candidates. The program does not remove files from disk.
