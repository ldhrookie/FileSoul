#ifndef STATS_H
#define STATS_H

#include "file_node.h"

typedef struct {
    char extension[MAX_EXTENSION_LENGTH];
    int count;
    int deleteCandidateCount;
    int deletedCount;
    long long totalSize;
    long long deleteCandidateSize;
    long long deletedSize;
} ExtensionStat;

int countFiles(const FileNode* head);
int countDeleteCandidates(const FileNode* head);
int countDeletedFiles(const FileNode* head);
long long calculateReclaimableSize(const FileNode* head);
long long calculateDeletedSize(const FileNode* head);
double calculateAverageInterest(const FileNode* head);
void printStatistics(const FileNode* head);
void printExtensionStatistics(const FileNode* head);
void formatSize(long long bytes, char* buffer, int bufferSize);

#endif
