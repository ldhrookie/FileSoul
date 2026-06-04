#ifndef DELETE_ACTIONS_H
#define DELETE_ACTIONS_H

#include "file_node.h"

int isProtectedFile(const FileSoul* file);
int isInsideScanRoot(const char* scanRoot, const char* filePath);
int canDeleteFile(const FileSoul* file, const char* scanRoot, char* reason, int reasonSize);
int deleteSingleFile(FileSoul* file, const char* scanRoot);
int deleteCandidateFiles(FileNode* head, const char* scanRoot);
void printDeletePreview(const FileNode* head);

#endif
