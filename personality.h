#ifndef PERSONALITY_H
#define PERSONALITY_H

#include "file_node.h"

typedef double (*InterestCalculator)(const FileSoul* file);

FileType getFileType(const char* extension);
FileMood getFileMood(const FileSoul* file);
Personality getPersonality(FileType type, long long size, time_t modifiedTime);
double calculateBasicInterest(const FileSoul* file);
void generateDialogue(FileSoul* file);
void assignPersonality(FileSoul* file, InterestCalculator calculator);
void assignPersonalities(FileNode* head);

const char* getFileTypeName(FileType type);
const char* getPersonalityName(Personality personality);
const char* getFileMoodName(FileMood mood);

#endif
