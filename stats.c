#include <stdio.h>

#include "stats.h"

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
    printf("\n===== FileSoul 통계 =====\n");
    printf("전체 파일 수: %d\n", countFiles(head));
    printf("삭제 후보 파일 수: %d\n", countDeleteCandidates(head));
    printf("확보 가능 저장공간: %lld bytes\n", calculateReclaimableSize(head));
    printf("평균 관심 지수: %.2f\n", calculateAverageInterest(head));
    printf("확장자별 통계: TODO\n");
}
