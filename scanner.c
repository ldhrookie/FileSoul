#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "scanner.h"

void extractExtension(const char* filename, char* extension, int extensionSize) {
    const char* dot;

    if (extension == NULL || extensionSize <= 0) {
        return;
    }

    extension[0] = '\0';

    if (filename == NULL) {
        return;
    }

    dot = strrchr(filename, '.');
    if (dot == NULL || dot == filename || dot[1] == '\0') {
        return;
    }

    strncpy(extension, dot + 1, (size_t)extensionSize - 1U);
    extension[extensionSize - 1] = '\0';
}

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

static int endsWithTemporaryMarker(const char* filename) {
    size_t length;

    if (filename == NULL) {
        return 1;
    }

    length = strlen(filename);
    return length == 0 || filename[0] == '~' || filename[length - 1] == '~';
}

int shouldSkipFile(const char* filename, const char* extension) {
    if (filename == NULL || filename[0] == '\0' || endsWithTemporaryMarker(filename)) {
        return 1;
    }

    if (equalsIgnoreCase(filename, "filesoul.exe") ||
        equalsIgnoreCase(filename, "report.txt")) {
        return 1;
    }

    if (equalsIgnoreCase(extension, "exe") ||
        equalsIgnoreCase(extension, "o") ||
        equalsIgnoreCase(extension, "obj") ||
        equalsIgnoreCase(extension, "log") ||
        equalsIgnoreCase(extension, "tmp")) {
        return 1;
    }

    return 0;
}

#ifdef _WIN32
static time_t fileTimeToTimeT(FILETIME fileTime) {
    ULARGE_INTEGER value;
    const unsigned long long windowsToUnixEpoch = 116444736000000000ULL;

    value.LowPart = fileTime.dwLowDateTime;
    value.HighPart = fileTime.dwHighDateTime;

    if (value.QuadPart < windowsToUnixEpoch) {
        return 0;
    }

    return (time_t)((value.QuadPart - windowsToUnixEpoch) / 10000000ULL);
}

static void buildSearchPattern(const char* folderPath, char* pattern, int patternSize) {
    size_t length;

    if (folderPath == NULL || folderPath[0] == '\0') {
        folderPath = ".";
    }

    snprintf(pattern, (size_t)patternSize, "%s", folderPath);
    length = strlen(pattern);

    if (length > 0 && pattern[length - 1] != '\\' && pattern[length - 1] != '/') {
        strncat(pattern, "\\*", (size_t)patternSize - strlen(pattern) - 1U);
    } else {
        strncat(pattern, "*", (size_t)patternSize - strlen(pattern) - 1U);
    }
}

static void buildFullPath(const char* folderPath, const char* filename, char* fullPath, int fullPathSize) {
    size_t length;

    snprintf(fullPath, (size_t)fullPathSize, "%s", folderPath != NULL && folderPath[0] != '\0' ? folderPath : ".");
    length = strlen(fullPath);

    if (length > 0 && fullPath[length - 1] != '\\' && fullPath[length - 1] != '/') {
        strncat(fullPath, "\\", (size_t)fullPathSize - strlen(fullPath) - 1U);
    }

    strncat(fullPath, filename, (size_t)fullPathSize - strlen(fullPath) - 1U);
}
#endif

int scanDirectory(const char* folderPath, FileNode** head) {
#ifdef _WIN32
    char pattern[MAX_PATH_LENGTH + 4];
    WIN32_FIND_DATAA findData;
    HANDLE handle;
    int count = 0;

    if (head == NULL) {
        return 0;
    }

    buildSearchPattern(folderPath, pattern, sizeof(pattern));

    handle = FindFirstFileA(pattern, &findData);
    if (handle == INVALID_HANDLE_VALUE) {
        printf("폴더를 열 수 없습니다: %s\n", folderPath != NULL ? folderPath : ".");
        return 0;
    }

    do {
        char extension[MAX_EXTENSION_LENGTH];
        char fullPath[MAX_PATH_LENGTH];
        long long size;
        time_t modifiedTime;
        FileNode* node;

        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            continue;
        }

        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
            continue;
        }

        extractExtension(findData.cFileName, extension, sizeof(extension));
        if (shouldSkipFile(findData.cFileName, extension)) {
            continue;
        }

        buildFullPath(folderPath, findData.cFileName, fullPath, sizeof(fullPath));
        size = ((long long)findData.nFileSizeHigh << 32) | (long long)findData.nFileSizeLow;
        modifiedTime = fileTimeToTimeT(findData.ftLastWriteTime);

        node = createFileNode(fullPath, findData.cFileName, extension, size, modifiedTime);
        if (node != NULL) {
            appendFileNode(head, node);
            ++count;
        }
    } while (FindNextFileA(handle, &findData) != 0);

    FindClose(handle);
    return count;
#else
    (void)folderPath;
    (void)head;
    printf("현재 폴더 스캔은 Windows 환경에서만 지원합니다.\n");
    return 0;
#endif
}
