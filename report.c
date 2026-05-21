#include <stdio.h>
#include "report.h"
#include "personality.h"

void writeReport(FileNode* head, const char* filename) {
    FILE* fp;
    FileNode* current = head;

    fp = fopen(filename, "w");

    if (fp == NULL) {
        printf("report.txt 파일을 생성할 수 없습니다.\n");
        return;
    }

    fprintf(fp, "=====================================\n");
    fprintf(fp, "           FileSoul Report\n");
    fprintf(fp, "      파일들의 작은 자기소개서\n");
    fprintf(fp, "=====================================\n\n");

    while (current != NULL) {
        fprintf(fp, "[파일명] %s\n", current->name);
        fprintf(fp, "[확장자] %s\n", current->extension);
        fprintf(fp, "[크기] %lld bytes\n", current->size);
        fprintf(fp, "[종류] %s\n", getFileTypeName(current->type));
        fprintf(fp, "[성격] %s\n", getPersonalityName(current->personality));
        fprintf(fp, "[대사] %s\n", current->dialogue);
        fprintf(fp, "\n-------------------------------------\n\n");

        current = current->next;
    }

    fclose(fp);
}