#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#endif

#include "personality.h"
#include "report.h"
#include "stats.h"

static FILE* openReportFile(const char* filename) {
    FILE* fp;

#ifdef _WIN32
    _mkdir("results");
    _mkdir("results/reports");
#endif

    fp = fopen(filename, "w");
    if (fp != NULL) {
        return fp;
    }

    printf("보고서 파일을 열지 못했습니다: %s\n", filename);
    printf("대신 report.txt에 저장을 시도합니다.\n");
    return fopen("report.txt", "w");
}

void writeReport(FileNode* head, const char* filename) {
    FILE* fp;
    FileNode* current = head;

    if (filename == NULL) {
        filename = "results/reports/report.txt";
    }

    fp = openReportFile(filename);

    if (fp == NULL) {
        printf("보고서를 생성하지 못했습니다.\n");
        return;
    }

    fprintf(fp, "=====================================\n");
    fprintf(fp, "           FileSoul Report\n");
    fprintf(fp, "=====================================\n\n");

    fprintf(fp, "전체 파일 수: %d\n", countFiles(head));
    fprintf(fp, "삭제 후보 파일 수: %d\n", countDeleteCandidates(head));
    fprintf(fp, "확보 가능 저장공간: %lld bytes\n", calculateReclaimableSize(head));
    fprintf(fp, "평균 관심 지수: %.2f\n\n", calculateAverageInterest(head));

    while (current != NULL) {
        const FileSoul* file = &current->data;

        fprintf(fp, "[파일명] %s\n", file->name);
        fprintf(fp, "[경로] %s\n", file->path);
        fprintf(fp, "[확장자] %s\n", file->extension[0] != '\0' ? file->extension : "(없음)");
        fprintf(fp, "[크기] %lld bytes\n", file->size);
        fprintf(fp, "[종류] %s\n", getFileTypeName(file->type));
        fprintf(fp, "[기분] %s\n", getFileMoodName(file->mood));
        fprintf(fp, "[성격] %s\n", getPersonalityName(file->personality));
        fprintf(fp, "[관심 지수] %.2f\n", file->interest);
        fprintf(fp, "[대사] %s\n", file->dialogue);
        fprintf(fp, "[사용자 선택] %s\n", getChoiceName(file->choice));
        fprintf(fp, "[삭제 후보] %s\n", file->deleteCandidate ? "예" : "아니오");
        fprintf(fp, "\n-------------------------------------\n\n");

        current = current->next;
    }

    fclose(fp);
    printf("보고서 저장 완료: %s\n", filename);
}
