#ifndef FILE_NODE_H
#define FILE_NODE_H

#define MAX_NAME_LENGTH 256
#define MAX_EXTENSION_LENGTH 32
#define MAX_DIALOGUE_LENGTH 256

typedef enum {
    TYPE_TEXT,
    TYPE_IMAGE,
    TYPE_CODE,
    TYPE_DOCUMENT,
    TYPE_EXECUTABLE,
    TYPE_UNKNOWN
} FileType;

typedef enum {
    PERSONALITY_DILIGENT,
    PERSONALITY_LAZY,
    PERSONALITY_HEAVY,
    PERSONALITY_OLD,
    PERSONALITY_DANGEROUS,
    PERSONALITY_MYSTERIOUS
} Personality;

typedef struct FileNode {
    char name[MAX_NAME_LENGTH];
    char extension[MAX_EXTENSION_LENGTH];
    long long size;

    FileType type;
    Personality personality;
    char dialogue[MAX_DIALOGUE_LENGTH];

    struct FileNode* next;
} FileNode;

FileNode* createFileNode(const char* name, const char* extension, long long size);
void appendFileNode(FileNode** head, FileNode* newNode);
void printFileList(FileNode* head);
void freeFileList(FileNode* head);

#endif