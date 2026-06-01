#include <stdio.h>
#include <string.h>
#include <time.h>

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

static void formatTimeValue(time_t value, char* buffer, int bufferSize) {
    struct tm* timeInfo;

    if (buffer == NULL || bufferSize <= 0) {
        return;
    }

    if (value <= 0) {
        snprintf(buffer, (size_t)bufferSize, "unknown");
        return;
    }

    timeInfo = localtime(&value);
    if (timeInfo == NULL) {
        snprintf(buffer, (size_t)bufferSize, "unknown");
        return;
    }

    strftime(buffer, (size_t)bufferSize, "%Y-%m-%d %H:%M:%S", timeInfo);
}

static void writeExtensionSummary(FILE* fp, const FileNode* head) {
    ExtensionStat stats[64];
    int used = 0;
    int i;

    memset(stats, 0, sizeof(stats));

    while (head != NULL) {
        const char* extension = head->data.extension[0] != '\0' ? head->data.extension : "(none)";
        int index = -1;
        for (i = 0; i < used; ++i) {
            if (strcmp(stats[i].extension, extension) == 0) {
                index = i;
                break;
            }
        }
        if (index < 0 && used < 64) {
            index = used++;
            snprintf(stats[index].extension, sizeof(stats[index].extension), "%s", extension);
        }
        if (index >= 0) {
            ++stats[index].count;
            stats[index].totalSize += head->data.size;
            if (head->data.deleteCandidate) {
                ++stats[index].deleteCandidateCount;
                stats[index].deleteCandidateSize += head->data.size;
            }
            if (head->data.deleted) {
                ++stats[index].deletedCount;
                stats[index].deletedSize += head->data.size;
            }
        }
        head = head->next;
    }

    fprintf(fp, "확장자별 통계\n");
    for (i = 0; i < used; ++i) {
        fprintf(fp, "- %s: files=%d, total=%lld bytes, candidates=%d/%lld bytes, deleted=%d/%lld bytes\n",
                stats[i].extension,
                stats[i].count,
                stats[i].totalSize,
                stats[i].deleteCandidateCount,
                stats[i].deleteCandidateSize,
                stats[i].deletedCount,
                stats[i].deletedSize);
    }
}

void writeReport(FileNode* head, const char* filename, const char* scanRoot) {
    FILE* fp;
    FileNode* current = head;
    char reportTime[64];
    time_t now = time(NULL);

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

    formatTimeValue(now, reportTime, sizeof(reportTime));
    fprintf(fp, "보고서 생성 시각: %s\n", reportTime);
    fprintf(fp, "스캔 폴더: %s\n", scanRoot != NULL ? scanRoot : "(unknown)");
    fprintf(fp, "전체 파일 수: %d\n", countFiles(head));
    fprintf(fp, "삭제 후보 파일 수: %d\n", countDeleteCandidates(head));
    fprintf(fp, "실제 삭제 파일 수: %d\n", countDeletedFiles(head));
    fprintf(fp, "확보 가능 저장공간: %lld bytes\n", calculateReclaimableSize(head));
    fprintf(fp, "실제 확보한 저장공간: %lld bytes\n", calculateDeletedSize(head));
    fprintf(fp, "평균 관심 지수: %.2f\n\n", calculateAverageInterest(head));
    writeExtensionSummary(fp, head);
    fprintf(fp, "\n");

    while (current != NULL) {
        const FileSoul* file = &current->data;
        char modified[64];

        formatTimeValue(file->modifiedTime, modified, sizeof(modified));

        fprintf(fp, "[파일명] %s\n", file->name);
        fprintf(fp, "[경로] %s\n", file->path);
        fprintf(fp, "[확장자] %s\n", file->extension[0] != '\0' ? file->extension : "(없음)");
        fprintf(fp, "[크기] %lld bytes\n", file->size);
        fprintf(fp, "[수정 시각] %s\n", modified);
        fprintf(fp, "[종류] %s\n", getFileTypeName(file->type));
        fprintf(fp, "[기분] %s\n", getFileMoodName(file->mood));
        fprintf(fp, "[성격] %s\n", getPersonalityName(file->personality));
        fprintf(fp, "[관심 지수] %.2f\n", file->interest);
        fprintf(fp, "[대사] %s\n", file->dialogue);
        fprintf(fp, "[사용자 선택] %s\n", getChoiceName(file->choice));
        fprintf(fp, "[삭제 후보] %s\n", file->deleteCandidate ? "예" : "아니오");
        fprintf(fp, "[실제 삭제] %s\n", file->deleted ? "예" : "아니오");
        fprintf(fp, "[삭제 실패] %s\n", file->deleteFailed ? "예" : "아니오");
        fprintf(fp, "[삭제 메시지] %s\n", file->deleteMessage);
        fprintf(fp, "\n-------------------------------------\n\n");

        current = current->next;
    }

    fclose(fp);
    printf("보고서 저장 완료: %s\n", filename);
}
