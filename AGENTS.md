# FileSoul Agent Guide

This repository is the GitHub-connected FileSoul project.

## Basic Rules

- Real file deletion is allowed only through the explicit guarded deletion flow.
- Files must first be marked as delete candidates.
- The program must show a deletion preview and require the user to type `DELETE` before calling `remove()`.
- Protected files, directories, repository files, build artifacts, and files outside the scanned root must never be deleted.
- Routine tasks must not remove files from the repository or user workspace except through this guarded program behavior.
- Preserve teammate code and history first.
- Do not move root C files or perform large refactors unless explicitly requested.
- Use the C standard library first; do not add external dependencies without approval.
- Handle memory allocation, file I/O, and input failures clearly.
- Free memory allocated with `malloc` before program exit.

## Working Log Rules

- Update `working_log.md` after every meaningful task.
- Log the intent of the task, the important commands, the practical changes, verification results, and notes for the next worker.
- Keep command lists short. Record only commands that matter for understanding the work.
- Before final reporting, check whether `working_log.md` reflects the current task.

## Git Rules

- Do not run `git push`, `git reset --hard`, `git clean -fd`, force push, rebase, or merge unless the user explicitly requests it.
- Do not work directly on `main` when making changes. Use a task branch.
- The remote repository is `https://github.com/ldhrookie/FileSoul.git`.
- Because teammates may work remotely, run `git fetch origin` and check status before and after integration work.

## Workspace Rules

- `C:\Users\LG\C_Programing` is the user's personal development workspace.
- `C:\Users\LG\C_Programing\FileSoul` is the actual GitHub-connected repository.
- Git commands, build verification, commits, and final integration must be performed inside `FileSoul`.
- The parent `src` folder can contain local development versions made by the user.
- Do not ignore the parent `src` folder, but do not copy it wholesale into the repository.
- Compare local development files with the current repository before copying.
- Do not overwrite teammate code without explicit user approval.

## Build Artifact Rules

- `filesoul.exe` is a build artifact and is no longer tracked in Git.
- Do not commit build outputs, object files, generated reports, or temporary delete-demo files.
- Keep `filesoul.exe`, `*.exe`, `*.o`, `*.obj`, `*.log`, report outputs, and `tests/delete_demo/` in `.gitignore`.
- After running `gcc *.c -o filesoul.exe`, `filesoul.exe` should remain ignored and absent from `git status`.

## Build

The current GitHub code builds from root-level `.c` and `.h` files.

```sh
gcc *.c -o filesoul.exe
```

The `src/` folder remains a preparation area and is not part of the current build.
