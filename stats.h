#ifndef STATS_H
#define STATS_H

#include "file_node.h"

int countFiles(const FileNode* head);
int countDeleteCandidates(const FileNode* head);
long long calculateReclaimableSize(const FileNode* head);
double calculateAverageInterest(const FileNode* head);
void printStatistics(const FileNode* head);

#endif
