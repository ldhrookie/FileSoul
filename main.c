#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "delete_actions.h"
#include "console_io.h"
#include "dialogue_view.h"
#include "file_node.h"
#include "personality.h"
#include "report.h"
#include "scanner.h"
#include "stats.h"

static void trimNewline(char* text) {
    size_t length;

    if (text == NULL) {
        return;
    }

    length = strlen(text);
    if (length > 0 && text[length - 1] == '\n') {
        text[length - 1] = '\0';
    }
}

static void addSampleFiles(FileNode** head) {
    appendFileNode(head, createSampleFileNode("homework.c", "c", 3200));
    appendFileNode(head, createSampleFileNode("old_photo.png", "png", 2480000));
    appendFileNode(head, createSampleFileNode("mysterious.tmp", "tmp", 800));
    appendFileNode(head, createSampleFileNode("final_report.docx", "docx", 145000));
    appendFileNode(head, createSampleFileNode("game.exe", "exe", 7300000));
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

static int askYesNo(const char* prompt) {
    char buffer[32];

    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }

    return buffer[0] == 'y' || buffer[0] == 'Y';
}

static int askDialogueLimit(int totalFiles) {
    char buffer[32];
    long value;
    char* end;

    printf("처음 바로 추천할 파일 개수입니다. 남은 파일은 타이머가 끝날 때마다 1개씩 추가 추천됩니다.\n");

    printf("대화할 파일 개수를 입력하세요. 빈 입력은 10개입니다: ");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return totalFiles < 10 ? totalFiles : 10;
    }

    if (buffer[0] == '\n' || buffer[0] == '\0') {
        return totalFiles < 10 ? totalFiles : 10;
    }

    value = strtol(buffer, &end, 10);
    if (end == buffer || value <= 0) {
        value = 10;
    }

    if (value > totalFiles) {
        value = totalFiles;
    }

    return (int)value;
}

static int confirmDelete(void) {
    char buffer[64];

    printf("실제 삭제를 진행하려면 DELETE를 정확히 입력하세요: ");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }

    trimNewline(buffer);
    return strcmp(buffer, "DELETE") == 0;
}

static void printLlmSetupStatus(void) {
    const char* apiKey = getenv("OPENAI_API_KEY");
    const char* model = getenv("FILESOUL_LLM_MODEL");
    const char* verificationFailed = getenv("FILESOUL_LLM_VERIFICATION_FAILED");

    if (verificationFailed != NULL && verificationFailed[0] != '\0') {
        printf("LLM inactive: startup API verification failed. Local dialogue is the final fallback.\n");
        return;
    }

    if (apiKey == NULL || apiKey[0] == '\0') {
        printf("LLM inactive: no verified API key. .\\run_filesoul.cmd can ask for one and test it safely.\n");
        return;
    }

    printf("LLM verification pending | model: %s\n",
           model != NULL && model[0] != '\0' ? model : "gpt-4.1-mini");
    printf("LLM is marked active only after a successful API response is applied to a file dialogue.\n");
}

#ifdef _WIN32
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
#endif

static int readFolderPath(char* folderPath, int folderPathSize) {
    char currentDirectory[MAX_PATH_LENGTH];

    if (folderPath == NULL || folderPathSize <= 0) {
        return 0;
    }

#ifdef _WIN32
    {
        wchar_t wideCurrentDirectory[MAX_PATH_LENGTH];

        if (GetCurrentDirectoryW((DWORD)(sizeof(wideCurrentDirectory) / sizeof(wideCurrentDirectory[0])),
                                 wideCurrentDirectory) == 0 ||
            !wideToUtf8(wideCurrentDirectory, currentDirectory, sizeof(currentDirectory))) {
            snprintf(currentDirectory, sizeof(currentDirectory), ".");
        }
    }
#else
    snprintf(currentDirectory, sizeof(currentDirectory), ".");
#endif

#ifdef _WIN32
    if (currentDirectory[0] == '\0') {
        snprintf(currentDirectory, sizeof(currentDirectory), ".");
    }
#endif

    printf("\n현재 작업 폴더: %s\n", currentDirectory);
    printf("검사할 폴더 경로를 입력하세요.\n");
    printf("- 빈 입력: 현재 작업 폴더 검사\n");
    printf("- sample 입력: 샘플 데이터로 데모 실행\n");
    printf("경로: ");

    if (fgets(folderPath, (size_t)folderPathSize, stdin) == NULL) {
        return 0;
    }

    trimNewline(folderPath);
    if (folderPath[0] == '\0') {
        snprintf(folderPath, (size_t)folderPathSize, ".");
    }

    return 1;
}

int main(void) {
    char folderPath[MAX_PATH_LENGTH];
    FileNode* head = NULL;
    int scannedCount;
    int sampleMode = 0;
    int allowRealDelete;
    int dialogueLimit;

    setlocale(LC_ALL, "");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    printf("FileSoul - 파일들의 목소리\n");
    printf("주의: 실제 삭제는 기본적으로 꺼져 있으며, DELETE 확인 후에만 실행됩니다.\n");
    printLlmSetupStatus();

    allowRealDelete = askYesNo("실제 파일 삭제 기능을 사용할까요? [y/N]: ");

    while (1) {
        if (!readFolderPath(folderPath, sizeof(folderPath))) {
            printf("\n입력이 종료되어 작업을 중단합니다.\n");
            return 1;
        }

        if (equalsIgnoreCase(folderPath, "sample")) {
            printf("샘플 데이터로 데모를 실행합니다.\n");
            addSampleFiles(&head);
            sampleMode = 1;
            break;
        }

        scannedCount = scanDirectory(folderPath, &head);
        if (scannedCount > 0) {
            printf("%d개 파일을 스캔했습니다.\n", scannedCount);
            break;
        }

        printf("스캔된 파일이 없습니다. 실제 폴더 경로를 다시 입력하거나 sample을 입력하세요.\n");
    }

    assignPersonalities(head);
    sortFileListByInterest(head);
    printFileList(head);
    dialogueLimit = askDialogueLimit(countFileNodes(head));
    showPopupDialoguesLimited(head, dialogueLimit);
    printStatistics(head);
    printDeletePreview(head);

    if (allowRealDelete && countDeleteCandidates(head) > 0) {
        if (confirmDelete()) {
            int deleted = deleteCandidateFiles(head, folderPath);
            printf("실제 삭제 완료: %d개 파일\n", deleted);
            printStatistics(head);
        } else {
            printf("DELETE 확인 문구가 일치하지 않아 실제 삭제를 건너뜁니다.\n");
        }
    } else if (!allowRealDelete) {
        printf("실제 삭제 기능이 꺼져 있어 파일을 삭제하지 않습니다.\n");
    }

    writeReport(head, "results/reports/report.txt", sampleMode ? "sample" : folderPath);
    freeFileList(head);

    printf("FileSoul 작업이 끝났습니다.\n");
    return 0;
}
