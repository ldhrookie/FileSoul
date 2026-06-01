#include "stats.h"

#include <stdio.h>

StatsSummary stats_calculate(const FileNode *head)
{
    StatsSummary summary = {0, 0, 0, 0.0};
    double interest_total = 0.0;
    const FileNode *current = head;

    while (current != NULL) {
        ++summary.total_files;
        interest_total += current->data.interest_score;

        if (current->data.is_delete_candidate) {
            ++summary.delete_candidate_count;
            summary.reclaimable_bytes += current->data.size_bytes;
        }

        current = current->next;
    }

    if (summary.total_files > 0) {
        summary.average_interest = interest_total / (double)summary.total_files;
    }

    return summary;
}

void stats_print(const StatsSummary *summary)
{
    if (summary == NULL) {
        return;
    }

    printf("\n전체 통계\n");
    printf("전체 파일 수: %zu\n", summary->total_files);
    printf("삭제 후보 파일 수: %zu\n", summary->delete_candidate_count);
    printf("확보 가능 저장공간: %llu bytes\n", summary->reclaimable_bytes);
    printf("평균 관심 지수: %.2f\n", summary->average_interest);
}

