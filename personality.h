#ifndef PERSONALITY_H
#define PERSONALITY_H

#include "file_node.h"

FileType getFileType(const char* extension);
Personality getPersonality(FileType type, long long size);
void generateDialogue(FileNode* file);
void assignPersonalities(FileNode* head);

const char* getFileTypeName(FileType type);
const char* getPersonalityName(Personality personality);

#endif