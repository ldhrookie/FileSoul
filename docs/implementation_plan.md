# FileSoul Implementation Plan

## Step 1: Keep Current Team Code Buildable

- Keep the root-level `.c/.h` layout for now.
- Do not move root files into `src/` yet.
- Preserve teammate code and make only minimal fixes needed for buildability.

## Step 2: Integrate Useful Local Development Pieces

- Treat `C:\Users\LG\C_Programing\src` as a local development version, not disposable scratch code.
- Compare local files with current repository code before copying.
- Copy only low-conflict preparation modules into repository `src/`.
- Do not copy local `main.c`, `report.c/h`, `persona.c/h`, or `dialog.c/h` over team code.

## Step 3: Prepare Data Structure Cleanup

- Introduce `FileSoul` as the file data structure.
- Keep `FileNode` as the linked list node.
- Move from the current combined `FileNode` design only after the team agrees on the transition.

## Step 4: Add Folder Scanning

- Ask the user for a folder path.
- Scan regular files in the selected folder first.
- Store file name, extension, size, path, and modified time.

## Step 5: Add Persona, Mood, and Interest Score

- Classify file types by extension.
- Add mood based on file size and modified time.
- Use a function pointer for interest score calculation.

## Step 6: Add Interactive Choices and Reports

- Show choices: open, keep, delete candidate, ignore.
- Save user choice results.
- Do not delete files.
- Write final reports to `results/reports/report.txt`.

## Step 7: Build Policy

- Continue validating with `gcc *.c -o filesoul.exe` while root-level files are the active build.
- Restore `filesoul.exe` after builds unless the user explicitly wants to commit the executable.
