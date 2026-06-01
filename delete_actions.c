#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "delete_actions.h"
#include "stats.h"

static int equalsIgnoreCase(const char* left, const char* right) {
    while (left != NULL && right != NULL && *left != '\0' && *right != '\0') {
        char a = *left;
        char b = *right;
        if (a >= 'A' && a <= 'Z') {
            a = (char)(a - 'A' + 'a');
        }
        if (b >= 'A' && b <= 'Z') {
            b = (char)(b - 'A' + 'a');
        }
        if (a != b) {
            return 0;
        }
        ++left;
        ++right;
    }

    return left != NULL && right != NULL && *left == '\0' && *right == '\0';
}

static int containsText(const char* text, const char* pattern) {
    return text != NULL && pattern != NULL && strstr(text, pattern) != NULL;
}

static void setDeleteMessage(FileSoul* file, int deleted, int failed, const char* message) {
    if (file == NULL) {
        return;
    }

    file->deleted = deleted;
    file->deleteFailed = failed;
    snprintf(file->deleteMessage, sizeof(file->deleteMessage), "%s", message != NULL ? message : "");
}

static int isSourceFile(const FileSoul* file) {
    if (file == NULL) {
        return 0;
    }

    return equalsIgnoreCase(file->extension, "c") ||
           equalsIgnoreCase(file->extension, "h") ||
           equalsIgnoreCase(file->extension, "cpp");
}

int isProtectedFile(const FileSoul* file) {
    if (file == NULL || file->path[0] == '\0' || file->name[0] == '\0') {
        return 1;
    }

    if (containsText(file->path, "..") || containsText(file->path, ".git\\") ||
        containsText(file->path, ".git/")) {
        return 1;
    }

    if (equalsIgnoreCase(file->name, "filesoul.exe") ||
        equalsIgnoreCase(file->name, ".gitignore") ||
        equalsIgnoreCase(file->name, "AGENTS.md") ||
        equalsIgnoreCase(file->name, "README.md") ||
        equalsIgnoreCase(file->name, "TASKS.md") ||
        equalsIgnoreCase(file->name, "working_log.md") ||
        equalsIgnoreCase(file->name, "report.txt")) {
        return 1;
    }

    if (equalsIgnoreCase(file->extension, "exe") ||
        equalsIgnoreCase(file->extension, "dll") ||
        equalsIgnoreCase(file->extension, "sys") ||
        equalsIgnoreCase(file->extension, "bat") ||
        equalsIgnoreCase(file->extension, "cmd") ||
        equalsIgnoreCase(file->extension, "ps1")) {
        return 1;
    }

    if (isSourceFile(file)) {
        return 1;
    }

    if (containsText(file->path, "results\\reports\\report.txt") ||
        containsText(file->path, "results/reports/report.txt")) {
        return 1;
    }

    return 0;
}

int isInsideScanRoot(const char* scanRoot, const char* filePath) {
#ifdef _WIN32
    char rootFull[MAX_PATH_LENGTH];
    char fileFull[MAX_PATH_LENGTH];
    size_t rootLength;

    if (scanRoot == NULL || filePath == NULL || filePath[0] == '\0') {
        return 0;
    }

    if (_fullpath(rootFull, scanRoot, sizeof(rootFull)) == NULL ||
        _fullpath(fileFull, filePath, sizeof(fileFull)) == NULL) {
        return 0;
    }

    rootLength = strlen(rootFull);
    if (rootLength > 0 && rootFull[rootLength - 1] != '\\' && rootFull[rootLength - 1] != '/') {
        strncat(rootFull, "\\", sizeof(rootFull) - strlen(rootFull) - 1U);
        ++rootLength;
    }

    return _strnicmp(rootFull, fileFull, rootLength) == 0;
#else
    return scanRoot != NULL && filePath != NULL && strstr(filePath, scanRoot) == filePath;
#endif
}

int canDeleteFile(const FileSoul* file, const char* scanRoot) {
#ifdef _WIN32
    DWORD attributes;
#endif

    if (file == NULL) {
        return 0;
    }

    if (!file->deleteCandidate) {
        return 0;
    }

    if (isProtectedFile(file)) {
        return 0;
    }

    if (!isInsideScanRoot(scanRoot, file->path)) {
        return 0;
    }

#ifdef _WIN32
    attributes = GetFileAttributesA(file->path);
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return 0;
    }
#endif

    return 1;
}

int deleteSingleFile(FileSoul* file, const char* scanRoot) {
    if (file == NULL) {
        return 0;
    }

    if (!file->deleteCandidate) {
        setDeleteMessage(file, 0, 0, "Skipped: not a delete candidate");
        return 0;
    }

    if (isProtectedFile(file)) {
        setDeleteMessage(file, 0, 1, "Skipped: protected file");
        return 0;
    }

    if (!isInsideScanRoot(scanRoot, file->path)) {
        setDeleteMessage(file, 0, 1, "Skipped: outside scan root");
        return 0;
    }

    if (!canDeleteFile(file, scanRoot)) {
        setDeleteMessage(file, 0, 1, "Skipped: not a deletable regular file");
        return 0;
    }

    if (remove(file->path) == 0) {
        setDeleteMessage(file, 1, 0, "Deleted successfully");
        return 1;
    }

    setDeleteMessage(file, 0, 1, "Failed: remove() returned an error");
    return 0;
}

int deleteCandidateFiles(FileNode* head, const char* scanRoot) {
    int deleted = 0;

    while (head != NULL) {
        if (head->data.deleteCandidate) {
            deleted += deleteSingleFile(&head->data, scanRoot);
        }
        head = head->next;
    }

    return deleted;
}

void printDeletePreview(const FileNode* head) {
    int count = 0;
    long long bytes = 0;
    char sizeText[64];

    printf("\n===== Delete Candidate Preview =====\n");

    while (head != NULL) {
        if (head->data.deleteCandidate) {
            ++count;
            bytes += head->data.size;
            formatSize(head->data.size, sizeText, sizeof(sizeText));
            printf("- %s (%s) %s\n", head->data.name, sizeText,
                   isProtectedFile(&head->data) ? "[protected]" : "");
        }
        head = head->next;
    }

    formatSize(bytes, sizeText, sizeof(sizeText));
    printf("Candidates: %d\n", count);
    printf("Potential reclaimable size: %s\n", sizeText);
}
