#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "delete_actions.h"
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

int main(void) {
    char folderPath[MAX_PATH_LENGTH];
    FileNode* head = NULL;
    int scannedCount;
    int allowRealDelete;
    int dialogueLimit;

    setlocale(LC_ALL, "");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    printf("FileSoul - 파일들의 목소리\n");
    printf("주의: 실제 삭제는 기본적으로 꺼져 있으며, DELETE 확인 후에만 실행됩니다.\n");
    printf("검사할 폴더 경로를 입력하세요. 빈 입력은 현재 폴더입니다: ");

    if (fgets(folderPath, sizeof(folderPath), stdin) == NULL) {
        printf("\n입력이 종료되어 현재 폴더를 검사합니다.\n");
        snprintf(folderPath, sizeof(folderPath), ".");
    } else {
        trimNewline(folderPath);
        if (folderPath[0] == '\0') {
            snprintf(folderPath, sizeof(folderPath), ".");
        }
    }

    allowRealDelete = askYesNo("실제 파일 삭제 기능을 사용할까요? [y/N]: ");

    scannedCount = scanDirectory(folderPath, &head);
    if (scannedCount == 0) {
        printf("스캔된 파일이 없어 샘플 데이터로 진행합니다.\n");
        addSampleFiles(&head);
    } else {
        printf("%d개 파일을 스캔했습니다.\n", scannedCount);
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

    writeReport(head, "results/reports/report.txt", folderPath);
    freeFileList(head);

    printf("FileSoul 작업이 끝났습니다.\n");
    return 0;
}
