#include <stdio.h>
#include <string.h>

#include "stats.h"

#define MAX_EXTENSION_STATS 64

static int extensionMatches(const char* left, const char* right) {
    return strcmp(left != NULL ? left : "", right != NULL ? right : "") == 0;
}

void formatSize(long long bytes, char* buffer, int bufferSize) {
    double value = (double)bytes;
    const char* unit = "B";

    if (buffer == NULL || bufferSize <= 0) {
        return;
    }

    if (value >= 1024.0) {
        value /= 1024.0;
        unit = "KB";
    }
    if (value >= 1024.0) {
        value /= 1024.0;
        unit = "MB";
    }
    if (value >= 1024.0) {
        value /= 1024.0;
        unit = "GB";
    }

    if (strcmp(unit, "B") == 0) {
        snprintf(buffer, (size_t)bufferSize, "%lld B", bytes);
    } else {
        snprintf(buffer, (size_t)bufferSize, "%.1f %s", value, unit);
    }
}

int countFiles(const FileNode* head) {
    return countFileNodes(head);
}

int countDeleteCandidates(const FileNode* head) {
    int count = 0;

    while (head != NULL) {
        if (head->data.deleteCandidate) {
            ++count;
        }
        head = head->next;
    }

    return count;
}

int countDeletedFiles(const FileNode* head) {
    int count = 0;

    while (head != NULL) {
        if (head->data.deleted) {
            ++count;
        }
        head = head->next;
    }

    return count;
}

long long calculateReclaimableSize(const FileNode* head) {
    long long total = 0;

    while (head != NULL) {
        if (head->data.deleteCandidate) {
            total += head->data.size;
        }
        head = head->next;
    }

    return total;
}

long long calculateDeletedSize(const FileNode* head) {
    long long total = 0;

    while (head != NULL) {
        if (head->data.deleted) {
            total += head->data.size;
        }
        head = head->next;
    }

    return total;
}

double calculateAverageInterest(const FileNode* head) {
    double total = 0.0;
    int count = 0;

    while (head != NULL) {
        total += head->data.interest;
        ++count;
        head = head->next;
    }

    if (count == 0) {
        return 0.0;
    }

    return total / (double)count;
}

void printStatistics(const FileNode* head) {
    char reclaimable[64];
    char deleted[64];

    formatSize(calculateReclaimableSize(head), reclaimable, sizeof(reclaimable));
    formatSize(calculateDeletedSize(head), deleted, sizeof(deleted));

    printf("\n===== FileSoul 통계 =====\n");
    printf("전체 파일 수: %d\n", countFiles(head));
    printf("삭제 후보 파일 수: %d\n", countDeleteCandidates(head));
    printf("실제 삭제 파일 수: %d\n", countDeletedFiles(head));
    printf("확보 가능 저장공간: %s\n", reclaimable);
    printf("실제 확보한 저장공간: %s\n", deleted);
    printf("평균 관심 지수: %.2f\n", calculateAverageInterest(head));
    printExtensionStatistics(head);
}

static int findOrCreateExtensionStat(ExtensionStat* stats, int* used, const char* extension) {
    int i;
    const char* normalized = extension != NULL && extension[0] != '\0' ? extension : "(none)";

    for (i = 0; i < *used; ++i) {
        if (extensionMatches(stats[i].extension, normalized)) {
            return i;
        }
    }

    if (*used >= MAX_EXTENSION_STATS) {
        return -1;
    }

    snprintf(stats[*used].extension, sizeof(stats[*used].extension), "%s", normalized);
    stats[*used].count = 0;
    stats[*used].deleteCandidateCount = 0;
    stats[*used].deletedCount = 0;
    stats[*used].totalSize = 0;
    stats[*used].deleteCandidateSize = 0;
    stats[*used].deletedSize = 0;
    ++(*used);
    return *used - 1;
}

void printExtensionStatistics(const FileNode* head) {
    ExtensionStat stats[MAX_EXTENSION_STATS];
    int used = 0;
    int i;

    memset(stats, 0, sizeof(stats));

    while (head != NULL) {
        int index = findOrCreateExtensionStat(stats, &used, head->data.extension);
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

    printf("\n확장자별 통계\n");
    for (i = 0; i < used; ++i) {
        char total[64];
        char candidate[64];
        char deleted[64];

        formatSize(stats[i].totalSize, total, sizeof(total));
        formatSize(stats[i].deleteCandidateSize, candidate, sizeof(candidate));
        formatSize(stats[i].deletedSize, deleted, sizeof(deleted));
        printf("- %s: %d files, total %s, candidates %d/%s, deleted %d/%s\n",
               stats[i].extension,
               stats[i].count,
               total,
               stats[i].deleteCandidateCount,
               candidate,
               stats[i].deletedCount,
               deleted);
    }
}
