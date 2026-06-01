#include "file_soul.h"

#include <stdlib.h>
#include <string.h>

void filesoul_init(FileSoul *soul)
{
    if (soul == NULL) {
        return;
    }

    memset(soul, 0, sizeof(*soul));
    soul->type = FILE_TYPE_OTHER;
    soul->mood = FILE_MOOD_UNKNOWN;
    soul->user_choice = USER_CHOICE_NONE;
}

FileNode *filesoul_create_node(const FileSoul *soul)
{
    FileNode *node;

    if (soul == NULL) {
        return NULL;
    }

    node = (FileNode *)malloc(sizeof(FileNode));
    if (node == NULL) {
        return NULL;
    }

    node->data = *soul;
    node->next = NULL;
    return node;
}

void filesoul_append(FileNode **head, const FileSoul *soul)
{
    FileNode *node;
    FileNode *current;

    if (head == NULL || soul == NULL) {
        return;
    }

    node = filesoul_create_node(soul);
    if (node == NULL) {
        return;
    }

    if (*head == NULL) {
        *head = node;
        return;
    }

    current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = node;
}

void filesoul_free_list(FileNode *head)
{
    while (head != NULL) {
        FileNode *next = head->next;
        free(head);
        head = next;
    }
}

size_t filesoul_count_list(const FileNode *head)
{
    size_t count = 0;

    while (head != NULL) {
        ++count;
        head = head->next;
    }

    return count;
}

const char *filesoul_type_to_string(FileType type)
{
    switch (type) {
    case FILE_TYPE_DOCUMENT:
        return "document";
    case FILE_TYPE_IMAGE:
        return "image";
    case FILE_TYPE_CODE:
        return "code";
    case FILE_TYPE_EXECUTABLE:
        return "executable";
    case FILE_TYPE_ARCHIVE:
        return "archive";
    case FILE_TYPE_OTHER:
    default:
        return "other";
    }
}

const char *filesoul_mood_to_string(FileMood mood)
{
    switch (mood) {
    case FILE_MOOD_CALM:
        return "calm";
    case FILE_MOOD_LONELY:
        return "lonely";
    case FILE_MOOD_HEAVY:
        return "heavy";
    case FILE_MOOD_URGENT:
        return "urgent";
    case FILE_MOOD_UNKNOWN:
    default:
        return "unknown";
    }
}

const char *filesoul_choice_to_string(UserChoice choice)
{
    switch (choice) {
    case USER_CHOICE_OPEN:
        return "open";
    case USER_CHOICE_KEEP:
        return "keep";
    case USER_CHOICE_DELETE_CANDIDATE:
        return "delete candidate";
    case USER_CHOICE_IGNORE:
        return "ignore";
    case USER_CHOICE_NONE:
    default:
        return "none";
    }
}

