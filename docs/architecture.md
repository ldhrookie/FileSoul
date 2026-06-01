# FileSoul Architecture Notes

## Current Flow

1. `main.c` creates sample file nodes.
2. `appendFileNode` adds them to a singly linked list.
3. `assignPersonalities` classifies file type and assigns personality/dialogue.
4. `printFileList` prints the list.
5. `showPopupDialogues` prints the dialogue view.
6. `freeFileList` releases allocated nodes.

## Current Modules

- `main.c`: demo flow and sample data
- `file_node.c/.h`: current file record plus linked list node
- `personality.c/.h`: type classification, personality assignment, dialogue generation
- `dialogue_view.c/.h`: simple dialogue output
- `report.c/.h`: report writer function, not yet wired into `main.c`

## Data Structures

The current team code uses `FileNode` for both file data and linked list structure. This keeps the current demo simple and buildable.

The planned direction is to split responsibilities later:

- `FileSoul`: one file's data, mood, personality, dialogue, choice result, and metadata
- `FileNode`: linked list node that owns one `FileSoul`

## Local Development Integration

`C:\Users\LG\C_Programing` is the user's personal development workspace. `C:\Users\LG\C_Programing\FileSoul` is the GitHub-connected repository.

The parent workspace `src/` contains local development modules. Only low-conflict preparation files are copied into this repository's `src/` folder. The current root-level team code is not moved or overwritten in this step.

## Build Structure

The current build uses root-level C files:

```sh
gcc *.c -o filesoul.exe
```

The copied `src/` files are not part of the current build yet.

## Safety

Real file deletion is out of scope. Delete-related behavior should only mark files as delete candidates.
