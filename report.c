#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#endif

#include "personality.h"
#include "console_io.h"
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

    printf("보고서 파일을 열 수 없습니다: %s\n", filename);
    printf("대신 report.txt에 저장을 시도합니다.\n");
    return fopen("report.txt", "w");
}

static void formatTimeValue(time_t value, char* buffer, int bufferSize) {
    struct tm* timeInfo;

    if (buffer == NULL || bufferSize <= 0) {
        return;
    }

    if (value <= 0) {
        snprintf(buffer, (size_t)bufferSize, "알 수 없음");
        return;
    }

    timeInfo = localtime(&value);
    if (timeInfo == NULL) {
        snprintf(buffer, (size_t)bufferSize, "알 수 없음");
        return;
    }

    strftime(buffer, (size_t)bufferSize, "%Y-%m-%d %H:%M:%S", timeInfo);
}

static int compareExtensionStats(const ExtensionStat* left, const ExtensionStat* right) {
    int nameCompare;

    if (left->totalSize != right->totalSize) {
        return left->totalSize < right->totalSize ? 1 : -1;
    }

    if (left->count != right->count) {
        return left->count < right->count ? 1 : -1;
    }

    nameCompare = strcmp(left->extension, right->extension);
    if (nameCompare < 0) {
        return -1;
    }
    if (nameCompare > 0) {
        return 1;
    }
    return 0;
}

static void sortExtensionStats(ExtensionStat* stats, int used) {
    int swapped;

    if (stats == NULL || used <= 1) {
        return;
    }

    do {
        int i;

        swapped = 0;
        for (i = 0; i < used - 1; ++i) {
            if (compareExtensionStats(&stats[i], &stats[i + 1]) > 0) {
                ExtensionStat temp = stats[i];
                stats[i] = stats[i + 1];
                stats[i + 1] = temp;
                swapped = 1;
            }
        }
    } while (swapped);
}

static void writeExtensionSummary(FILE* fp, const FileNode* head) {
    ExtensionStat stats[64];
    int used = 0;
    int i;

    memset(stats, 0, sizeof(stats));

    while (head != NULL) {
        const char* extension = head->data.extension[0] != '\0' ? head->data.extension : "(없음)";
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

    sortExtensionStats(stats, used);

    fprintf(fp, "확장자별 통계\n");
    fprintf(fp, "%-12s %8s %12s %16s %18s\n",
            "확장자", "파일", "전체 크기", "삭제 후보", "후보 크기");
    for (i = 0; i < used; ++i) {
        char total[64];
        char candidate[64];

        formatSize(stats[i].totalSize, total, sizeof(total));
        formatSize(stats[i].deleteCandidateSize, candidate, sizeof(candidate));
        fprintf(fp, "%-12s %8d %12s %16d %18s\n",
                stats[i].extension,
                stats[i].count,
                total,
                stats[i].deleteCandidateCount,
                candidate);
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
        printf("보고서를 만들 수 없습니다.\n");
        return;
    }

    fprintf(fp, "=====================================\n");
    fprintf(fp, "           FileSoul 보고서\n");
    fprintf(fp, "=====================================\n\n");

    formatTimeValue(now, reportTime, sizeof(reportTime));
    fprintf(fp, "보고서 생성 시각: %s\n", reportTime);
    fprintf(fp, "스캔 폴더: %s\n", scanRoot != NULL ? scanRoot : "(알 수 없음)");
    fprintf(fp, "전체 파일 수: %d\n", countFiles(head));
    fprintf(fp, "삭제 후보 수: %d\n", countDeleteCandidates(head));
    fprintf(fp, "실제 삭제 수: %d\n", countDeletedFiles(head));
    fprintf(fp, "예상 확보 공간: %lld bytes\n", calculateReclaimableSize(head));
    fprintf(fp, "실제 확보 공간: %lld bytes\n", calculateDeletedSize(head));
    fprintf(fp, "평균 관심도: %.2f\n\n", calculateAverageInterest(head));
    writeExtensionSummary(fp, head);
    fprintf(fp, "\n");

    while (current != NULL) {
        const FileSoul* file = &current->data;
        char modified[64];
        char sizeText[64];

        formatTimeValue(file->modifiedTime, modified, sizeof(modified));
        formatSize(file->size, sizeText, sizeof(sizeText));

        fprintf(fp, "[파일] %s\n", file->name);
        fprintf(fp, "[경로] %s\n", file->path);
        fprintf(fp, "[확장자] %s\n", file->extension[0] != '\0' ? file->extension : "(없음)");
        fprintf(fp, "[크기] %s\n", sizeText);
        fprintf(fp, "[수정 시각] %s\n", modified);
        fprintf(fp, "[종류] %s\n", getFileTypeName(file->type));
        fprintf(fp, "[기분] %s\n", getFileMoodName(file->mood));
        fprintf(fp, "[성격] %s\n", getPersonalityName(file->personality));
        fprintf(fp, "[관심도] %.2f\n", file->interest);
        fprintf(fp, "[대사] %s\n", file->dialogue);
        fprintf(fp, "[사용자 선택] %s\n", getChoiceName(file->choice));
        fprintf(fp, "[삭제 후보] %s\n", file->deleteCandidate ? "예" : "아니오");
        fprintf(fp, "[삭제됨] %s\n", file->deleted ? "예" : "아니오");
        fprintf(fp, "[삭제 실패] %s\n", file->deleteFailed ? "예" : "아니오");
        fprintf(fp, "[삭제 메시지] %s\n", file->deleteMessage);
        fprintf(fp, "\n-------------------------------------\n\n");

        current = current->next;
    }

    fclose(fp);
    printf("보고서 저장 완료: %s\n", filename);
}
