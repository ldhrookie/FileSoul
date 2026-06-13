# FileSoul Architecture Notes

## Execution Flow

1. Print title and deletion safety notice.
2. Ask for the scan folder path and retry until a real scan succeeds or the user explicitly enters `sample`.
3. Ask whether guarded real deletion should be enabled.
4. Scan regular files in the folder.
5. Assign file type, mood, personality, dialogue, and interest score.
6. Sort by interest score.
7. Print file list.
8. Ask how many files to discuss.
9. Generate an LLM dialogue when configured, then show a floating message with action buttons.
10. Print statistics, including sorted extension statistics.
11. Print delete candidate preview.
12. If real deletion is enabled, require exact `DELETE` confirmation.
13. Delete only candidates that pass protection checks; store a detailed block reason when deletion is refused.
14. Print updated statistics.
15. Write `report.txt`.
16. Free the linked list.

## Core Data Structures

`FileSoul` stores one file's path, name, extension, size, modified time, type, mood, personality, interest, dialogue, user choice, delete candidate flag, delete result, and extra union data.

`FileNode` is the singly linked list node that owns one `FileSoul`.

## Modules

- `scanner.c/.h`: Windows folder scan and scan filters
- `console_io.c/.h`: UTF-8 formatting and direct UTF-16 Windows console output
- `personality.c/.h`: type, mood, personality, dialogue, interest score
- `llm_dialogue.c/.h`: Groq Chat Completions dialogue generation with local-dialogue fallback
- `dialogue_view.c/.h`: floating message choices, terminal fallback, and batch actions
- `delete_actions.c/.h`: guarded deletion, protected file checks, delete preview, block reasons
- `stats.c/.h`: counts, sizes, sorted extension statistics, size formatting
- `report.c/.h`: report output, sorted extension summary, delete messages, fallback report path
- `file_node.c/.h`: data structures and linked list helpers

## Guarded Deletion

The program never deletes during normal selection. It only sets `deleteCandidate`.

Actual deletion requires the startup opt-in, preview, exact `DELETE` confirmation, root containment check, protected file check, regular file check, and then `remove()`.
Blocked deletion attempts store the reason in `FileSoul.deleteMessage` so the terminal and report explain why the file was skipped.

## Floating Dialogue

On Windows, the dialogue view uses `TaskDialogIndirect` custom buttons so the file speaks and the user chooses an action in the same floating message. If the API key is configured, the LLM receives only file metadata and produces a short personality-driven Korean line. Full paths and file contents are not sent.
The external `filesoul.exe.manifest` activates Common Controls v6 for the custom task dialog.
Local fallback dialogue uses stable file metadata to choose distinct lines within each personality instead of repeating one fixed sentence.

## Build Structure

The active build remains root-level:

```sh
gcc *.c -o filesoul.exe
```

The repository `src/` folder is not compiled yet.
`filesoul.exe` and generated reports are ignored build/runtime outputs and should not be committed.
