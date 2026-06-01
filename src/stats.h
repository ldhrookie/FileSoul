#ifndef STATS_H
#define STATS_H

#include "file_soul.h"

typedef struct {
    size_t total_files;
    size_t delete_candidate_count;
    unsigned long long reclaimable_bytes;
    double average_interest;
} StatsSummary;

StatsSummary stats_calculate(const FileNode *head);
void stats_print(const StatsSummary *summary);

#endif

