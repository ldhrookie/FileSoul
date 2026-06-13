#include <stdio.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "scanner.h"
#include "console_io.h"
#include "string_utils.h"

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

static int utf8ToWide(const char* source, wchar_t* destination, int destinationSize) {
    int converted;

    if (destination == NULL || destinationSize <= 0) {
        return 0;
    }

    destination[0] = L'\0';
    if (source == NULL || source[0] == '\0') {
        source = ".";
    }

    converted = MultiByteToWideChar(CP_UTF8, 0, source, -1, destination, destinationSize);
    if (converted == 0) {
        converted = MultiByteToWideChar(CP_ACP, 0, source, -1, destination, destinationSize);
    }

    return converted != 0;
}

static int wideToUtf8(const wchar_t* source, char* destination, int destinationSize) {
    int converted;

    if (destination == NULL || destinationSize <= 0) {
        return 0;
    }

    destination[0] = '\0';
    if (source == NULL) {
        return 0;
    }

    converted = WideCharToMultiByte(CP_UTF8, 0, source, -1, destination, destinationSize, NULL, NULL);
    return converted != 0;
}

static void buildSearchPattern(const wchar_t* folderPath, wchar_t* pattern, int patternSize) {
    size_t length;

    if (pattern == NULL || patternSize <= 0) {
        return;
    }

    _snwprintf(pattern, (size_t)patternSize, L"%ls",
               folderPath != NULL && folderPath[0] != L'\0' ? folderPath : L".");
    pattern[patternSize - 1] = L'\0';
    length = wcslen(pattern);

    if (length > 0 && pattern[length - 1] != L'\\' && pattern[length - 1] != L'/') {
        wcsncat(pattern, L"\\*", (size_t)patternSize - wcslen(pattern) - 1U);
    } else {
        wcsncat(pattern, L"*", (size_t)patternSize - wcslen(pattern) - 1U);
    }
}

static void buildFullPath(const wchar_t* folderPath, const wchar_t* filename, wchar_t* fullPath, int fullPathSize) {
    size_t length;

    if (fullPath == NULL || fullPathSize <= 0) {
        return;
    }

    _snwprintf(fullPath, (size_t)fullPathSize, L"%ls",
               folderPath != NULL && folderPath[0] != L'\0' ? folderPath : L".");
    fullPath[fullPathSize - 1] = L'\0';
    length = wcslen(fullPath);

    if (length > 0 && fullPath[length - 1] != L'\\' && fullPath[length - 1] != L'/') {
        wcsncat(fullPath, L"\\", (size_t)fullPathSize - wcslen(fullPath) - 1U);
    }

    wcsncat(fullPath, filename, (size_t)fullPathSize - wcslen(fullPath) - 1U);
}
#endif

int scanDirectory(const char* folderPath, FileNode** head) {
#ifdef _WIN32
    wchar_t wideFolderPath[MAX_PATH_LENGTH];
    wchar_t pattern[MAX_PATH_LENGTH + 4];
    WIN32_FIND_DATAW findData;
    HANDLE handle;
    int count = 0;

    if (head == NULL) {
        return 0;
    }

    if (!utf8ToWide(folderPath, wideFolderPath, (int)(sizeof(wideFolderPath) / sizeof(wideFolderPath[0])))) {
        printf("폴더 경로를 Windows 형식으로 변환할 수 없습니다: %s\n", folderPath != NULL ? folderPath : ".");
        return 0;
    }

    buildSearchPattern(wideFolderPath, pattern, (int)(sizeof(pattern) / sizeof(pattern[0])));

    handle = FindFirstFileW(pattern, &findData);
    if (handle == INVALID_HANDLE_VALUE) {
        printf("폴더를 열 수 없습니다: %s\n", folderPath != NULL ? folderPath : ".");
        return 0;
    }

    do {
        char extension[MAX_EXTENSION_LENGTH];
        char fullPath[MAX_PATH_LENGTH];
        char fileName[MAX_NAME_LENGTH];
        wchar_t wideFullPath[MAX_PATH_LENGTH];
        long long size;
        time_t modifiedTime;
        FileNode* node;

        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            continue;
        }

        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0) {
            continue;
        }

        if (!wideToUtf8(findData.cFileName, fileName, sizeof(fileName))) {
            continue;
        }

        extractExtension(fileName, extension, sizeof(extension));
        if (shouldSkipFile(fileName, extension)) {
            continue;
        }

        buildFullPath(wideFolderPath, findData.cFileName, wideFullPath,
                      (int)(sizeof(wideFullPath) / sizeof(wideFullPath[0])));
        if (!wideToUtf8(wideFullPath, fullPath, sizeof(fullPath))) {
            continue;
        }
        size = ((long long)findData.nFileSizeHigh << 32) | (long long)findData.nFileSizeLow;
        modifiedTime = fileTimeToTimeT(findData.ftLastWriteTime);

        node = createFileNode(fullPath, fileName, extension, size, modifiedTime);
        if (node != NULL) {
            appendFileNode(head, node);
            ++count;
        }
    } while (FindNextFileW(handle, &findData) != 0);

    FindClose(handle);
    return count;
#else
    (void)folderPath;
    (void)head;
    printf("현재 폴더 스캔은 Windows 환경에서만 지원합니다.\n");
    return 0;
#endif
}
