#ifndef FILE_NODE_H
#define FILE_NODE_H

#include <time.h>

#define MAX_PATH_LENGTH 260
#define MAX_NAME_LENGTH 256
#define MAX_EXTENSION_LENGTH 32
#define MAX_DIALOGUE_LENGTH 256
#define MAX_DELETE_MESSAGE_LENGTH 256

typedef enum {
    TYPE_TEXT,
    TYPE_IMAGE,
    TYPE_CODE,
    TYPE_DOCUMENT,
    TYPE_EXECUTABLE,
    TYPE_ARCHIVE,
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

typedef enum {
    MOOD_CALM,
    MOOD_LONELY,
    MOOD_HEAVY,
    MOOD_URGENT,
    MOOD_UNKNOWN
} FileMood;

typedef enum {
    CHOICE_NONE,
    CHOICE_OPEN,
    CHOICE_KEEP,
    CHOICE_DELETE_CANDIDATE,
    CHOICE_IGNORE
} UserChoice;

typedef union {
    int codeLineCount;
    int imagePixelScore;
    int documentPageGuess;
    double extraScore;
} FileExtraInfo;

typedef struct {
    char path[MAX_PATH_LENGTH];
    char name[MAX_NAME_LENGTH];
    char extension[MAX_EXTENSION_LENGTH];
    long long size;
    time_t modifiedTime;

    FileType type;
    FileMood mood;
    Personality personality;
    double interest;

    char dialogue[MAX_DIALOGUE_LENGTH];
    UserChoice choice;
    int deleteCandidate;

    FileExtraInfo extra;

    int deleted;
    int deleteFailed;
    char deleteMessage[MAX_DELETE_MESSAGE_LENGTH];
} FileSoul;

typedef struct FileNode {
    FileSoul data;
    struct FileNode* next;
} FileNode;

FileNode* createFileNode(const char* path, const char* name, const char* extension, long long size, time_t modifiedTime);
FileNode* createSampleFileNode(const char* name, const char* extension, long long size);
void appendFileNode(FileNode** head, FileNode* newNode);
void printFileList(const FileNode* head);
void freeFileList(FileNode* head);
void sortFileListByInterest(FileNode* head);
int countFileNodes(const FileNode* head);
const char* getChoiceName(UserChoice choice);

#endif
