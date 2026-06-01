# FileSoul Architecture Notes

## Execution Flow

1. Print title and deletion safety notice.
2. Ask for the scan folder path.
3. Ask whether guarded real deletion should be enabled.
4. Scan regular files in the folder.
5. Assign file type, mood, personality, dialogue, and interest score.
6. Sort by interest score.
7. Print file list.
8. Ask how many files to discuss.
9. Run terminal dialogue choices, including batch choices.
10. Print statistics.
11. Print delete candidate preview.
12. If real deletion is enabled, require exact `DELETE` confirmation.
13. Delete only candidates that pass protection checks.
14. Print updated statistics.
15. Write `results/reports/report.txt`.
16. Free the linked list.

## Core Data Structures

`FileSoul` stores one file's path, name, extension, size, modified time, type, mood, personality, interest, dialogue, user choice, delete candidate flag, delete result, and extra union data.

`FileNode` is the singly linked list node that owns one `FileSoul`.

## Modules

- `scanner.c/.h`: Windows folder scan and scan filters
- `personality.c/.h`: type, mood, personality, dialogue, interest score
- `dialogue_view.c/.h`: terminal choices and batch actions
- `delete_actions.c/.h`: guarded deletion, protected file checks, delete preview
- `stats.c/.h`: counts, sizes, extension statistics, size formatting
- `report.c/.h`: report output and fallback report path
- `file_node.c/.h`: data structures and linked list helpers

## Guarded Deletion

The program never deletes during normal selection. It only sets `deleteCandidate`.

Actual deletion requires the startup opt-in, preview, exact `DELETE` confirmation, root containment check, protected file check, regular file check, and then `remove()`.

## Build Structure

The active build remains root-level:

```sh
gcc *.c -o filesoul.exe
```

The repository `src/` folder is not compiled yet.
