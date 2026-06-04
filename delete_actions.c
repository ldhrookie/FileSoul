#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "delete_actions.h"
#include "console_io.h"
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

static void copyReason(char* reason, int reasonSize, const char* message) {
    if (reason == NULL || reasonSize <= 0) {
        return;
    }

    snprintf(reason, (size_t)reasonSize, "%s", message != NULL ? message : "");
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

static int isDocumentFileName(const char* name) {
    return equalsIgnoreCase(name, "AGENTS.md") ||
           equalsIgnoreCase(name, "README.md") ||
           equalsIgnoreCase(name, "TASKS.md") ||
           equalsIgnoreCase(name, "architecture.md") ||
           equalsIgnoreCase(name, "implementation_plan.md") ||
           equalsIgnoreCase(name, "working_log.md");
}

static int getProtectedFileReason(const FileSoul* file, char* reason, int reasonSize) {
    if (file == NULL || file->path[0] == '\0' || file->name[0] == '\0') {
        copyReason(reason, reasonSize, "삭제 차단: 파일 정보가 완전하지 않습니다.");
        return 1;
    }

    if (containsText(file->path, "..")) {
        copyReason(reason, reasonSize, "삭제 차단: '..'가 포함된 경로는 안전하지 않습니다.");
        return 1;
    }

    if (containsText(file->path, ".git\\") || containsText(file->path, ".git/")) {
        copyReason(reason, reasonSize, "삭제 차단: Git 내부 파일은 삭제할 수 없습니다.");
        return 1;
    }

    if (equalsIgnoreCase(file->name, "filesoul.exe")) {
        copyReason(reason, reasonSize, "삭제 차단: filesoul.exe는 빌드 결과물입니다.");
        return 1;
    }

    if (isDocumentFileName(file->name)) {
        copyReason(reason, reasonSize, "삭제 차단: 프로젝트 문서 파일입니다.");
        return 1;
    }

    if (equalsIgnoreCase(file->name, ".gitignore") ||
        equalsIgnoreCase(file->name, ".gitattributes")) {
        copyReason(reason, reasonSize, "삭제 차단: 저장소 설정 파일은 보호됩니다.");
        return 1;
    }

    if (equalsIgnoreCase(file->name, "report.txt") ||
        containsText(file->path, "results\\reports\\report.txt") ||
        containsText(file->path, "results/reports/report.txt")) {
        copyReason(reason, reasonSize, "삭제 차단: 생성된 보고서 파일은 보호됩니다.");
        return 1;
    }

    if (equalsIgnoreCase(file->extension, "exe") ||
        equalsIgnoreCase(file->extension, "dll") ||
        equalsIgnoreCase(file->extension, "sys")) {
        copyReason(reason, reasonSize, "삭제 차단: 실행 파일 또는 시스템 파일은 보호됩니다.");
        return 1;
    }

    if (equalsIgnoreCase(file->extension, "bat") ||
        equalsIgnoreCase(file->extension, "cmd") ||
        equalsIgnoreCase(file->extension, "ps1")) {
        copyReason(reason, reasonSize, "삭제 차단: 스크립트 파일은 보호됩니다.");
        return 1;
    }

    if (isSourceFile(file)) {
        copyReason(reason, reasonSize, "삭제 차단: 프로젝트 소스 파일은 보호됩니다.");
        return 1;
    }

    return 0;
}

int isProtectedFile(const FileSoul* file) {
    char reason[MAX_DELETE_MESSAGE_LENGTH];

    return getProtectedFileReason(file, reason, sizeof(reason));
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

int canDeleteFile(const FileSoul* file, const char* scanRoot, char* reason, int reasonSize) {
#ifdef _WIN32
    DWORD attributes;
#endif

    copyReason(reason, reasonSize, "");

    if (file == NULL) {
        copyReason(reason, reasonSize, "삭제 차단: 파일 정보가 없습니다.");
        return 0;
    }

    if (!file->deleteCandidate) {
        copyReason(reason, reasonSize, "건너뜀: 삭제 후보로 등록된 파일이 아닙니다.");
        return 0;
    }

    if (getProtectedFileReason(file, reason, reasonSize)) {
        return 0;
    }

    if (!isInsideScanRoot(scanRoot, file->path)) {
        copyReason(reason, reasonSize, "삭제 차단: 스캔한 폴더 밖의 파일입니다.");
        return 0;
    }

#ifdef _WIN32
    attributes = GetFileAttributesA(file->path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        copyReason(reason, reasonSize, "삭제 차단: 파일 속성을 읽을 수 없습니다.");
        return 0;
    }

    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        copyReason(reason, reasonSize, "삭제 차단: 폴더는 삭제 대상이 아닙니다.");
        return 0;
    }
#endif

    copyReason(reason, reasonSize, "삭제 준비 완료.");
    return 1;
}

int deleteSingleFile(FileSoul* file, const char* scanRoot) {
    char reason[MAX_DELETE_MESSAGE_LENGTH];

    if (file == NULL) {
        return 0;
    }

    if (!canDeleteFile(file, scanRoot, reason, sizeof(reason))) {
        setDeleteMessage(file, 0, file->deleteCandidate ? 1 : 0, reason);
        return 0;
    }

    if (remove(file->path) == 0) {
        setDeleteMessage(file, 1, 0, "삭제 성공.");
        return 1;
    }

    setDeleteMessage(file, 0, 1, "삭제 실패: remove()가 오류를 반환했습니다.");
    return 0;
}

int deleteCandidateFiles(FileNode* head, const char* scanRoot) {
    int deleted = 0;

    while (head != NULL) {
        if (head->data.deleteCandidate) {
            deleted += deleteSingleFile(&head->data, scanRoot);
            if (head->data.deleteFailed) {
                printf("%s: %s\n", head->data.name, head->data.deleteMessage);
            }
        }
        head = head->next;
    }

    return deleted;
}

void printDeletePreview(const FileNode* head) {
    int count = 0;
    long long bytes = 0;
    char sizeText[64];

    printf("\n===== 삭제 후보 미리보기 =====\n");

    while (head != NULL) {
        if (head->data.deleteCandidate) {
            char reason[MAX_DELETE_MESSAGE_LENGTH];
            ++count;
            bytes += head->data.size;
            formatSize(head->data.size, sizeText, sizeof(sizeText));
            if (isProtectedFile(&head->data)) {
                getProtectedFileReason(&head->data, reason, sizeof(reason));
                printf("- %s (%s) [차단] %s\n", head->data.name, sizeText, reason);
            } else {
                printf("- %s (%s)\n", head->data.name, sizeText);
            }
        }
        head = head->next;
    }

    formatSize(bytes, sizeText, sizeof(sizeText));
    printf("삭제 후보 수: %d\n", count);
    printf("예상 확보 공간: %s\n", sizeText);
}
