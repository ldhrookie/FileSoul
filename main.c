#include <locale.h>
#include <stdio.h>
#include <string.h>

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

int main(void) {
    char folderPath[MAX_PATH_LENGTH];
    FileNode* head = NULL;
    int scannedCount;

    setlocale(LC_ALL, "");

    printf("FileSoul - 파이리의 외침\n");
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
    showPopupDialogues(head);
    printStatistics(head);
    writeReport(head, "results/reports/report.txt");
    freeFileList(head);

    printf("FileSoul 작업이 끝났습니다.\n");
    return 0;
}
