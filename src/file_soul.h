#ifndef FILE_SOUL_H
#define FILE_SOUL_H

#include <stddef.h>
#include <time.h>

#define FILESOUL_MAX_PATH 512
#define FILESOUL_MAX_NAME 256
#define FILESOUL_MAX_EXT 32
#define FILESOUL_MAX_PERSONALITY 128
#define FILESOUL_MAX_DIALOG 256

typedef enum {
    FILE_TYPE_DOCUMENT,
    FILE_TYPE_IMAGE,
    FILE_TYPE_CODE,
    FILE_TYPE_EXECUTABLE,
    FILE_TYPE_ARCHIVE,
    FILE_TYPE_OTHER
} FileType;

typedef enum {
    FILE_MOOD_CALM,
    FILE_MOOD_LONELY,
    FILE_MOOD_HEAVY,
    FILE_MOOD_URGENT,
    FILE_MOOD_UNKNOWN
} FileMood;

typedef enum {
    USER_CHOICE_NONE,
    USER_CHOICE_OPEN,
    USER_CHOICE_KEEP,
    USER_CHOICE_DELETE_CANDIDATE,
    USER_CHOICE_IGNORE
} UserChoice;

typedef union {
    int line_count;
    int image_score;
    int document_pages_estimate;
    int extra_score;
} FileExtraInfo;

typedef struct {
    char path[FILESOUL_MAX_PATH];
    char name[FILESOUL_MAX_NAME];
    char extension[FILESOUL_MAX_EXT];
    unsigned long long size_bytes;
    time_t modified_time;
    FileType type;
    FileMood mood;
    double interest_score;
    char personality[FILESOUL_MAX_PERSONALITY];
    char dialog[FILESOUL_MAX_DIALOG];
    UserChoice user_choice;
    int is_delete_candidate;
    FileExtraInfo extra;
} FileSoul;

typedef struct FileNode {
    FileSoul data;
    struct FileNode *next;
} FileNode;

void filesoul_init(FileSoul *soul);
FileNode *filesoul_create_node(const FileSoul *soul);
void filesoul_append(FileNode **head, const FileSoul *soul);
void filesoul_free_list(FileNode *head);
size_t filesoul_count_list(const FileNode *head);

const char *filesoul_type_to_string(FileType type);
const char *filesoul_mood_to_string(FileMood mood);
const char *filesoul_choice_to_string(UserChoice choice);

#endif

