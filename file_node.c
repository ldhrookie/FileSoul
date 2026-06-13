#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "file_node.h"
#include "console_io.h"
#include "personality.h"

static void copyText(char* destination, int destinationSize, const char* source) {
    if (destination == NULL || destinationSize <= 0) {
        return;
    }

    if (source == NULL) {
        destination[0] = '\0';
        return;
    }

    strncpy(destination, source, (size_t)destinationSize - 1U);
    destination[destinationSize - 1] = '\0';
}

static void initFileSoul(FileSoul* file) {
    if (file == NULL) {
        return;
    }

    memset(file, 0, sizeof(*file));
    file->type = TYPE_UNKNOWN;
    file->mood = MOOD_UNKNOWN;
    file->personality = PERSONALITY_MYSTERIOUS;
    file->choice = CHOICE_NONE;
    file->deleteCandidate = 0;
    file->interest = 0.0;
    file->deleted = 0;
    file->deleteFailed = 0;
    copyText(file->deleteMessage, MAX_DELETE_MESSAGE_LENGTH, "삭제 요청 없음");
}

FileNode* createFileNode(const char* path, const char* name, const char* extension, long long size, time_t modifiedTime) {
    FileNode* newNode = (FileNode*)malloc(sizeof(FileNode));

    if (newNode == NULL) {
        printf("메모리 할당에 실패했습니다.\n");
        return NULL;
    }

    initFileSoul(&newNode->data);
    copyText(newNode->data.path, MAX_PATH_LENGTH, path);
    copyText(newNode->data.name, MAX_NAME_LENGTH, name);
    copyText(newNode->data.extension, MAX_EXTENSION_LENGTH, extension);
    newNode->data.size = size;
    newNode->data.modifiedTime = modifiedTime;
    copyText(newNode->data.dialogue, MAX_DIALOGUE_LENGTH, "아직 내 말이 정해지지 않았어요.");
    newNode->next = NULL;

    return newNode;
}

FileNode* createSampleFileNode(const char* name, const char* extension, long long size) {
    char path[MAX_PATH_LENGTH];

    snprintf(path, sizeof(path), "sample/%s", name != NULL ? name : "unknown");
    return createFileNode(path, name, extension, size, time(NULL) - 60 * 60 * 24 * 30);
}

void appendFileNode(FileNode** head, FileNode* newNode) {
    FileNode* current;

    if (head == NULL || newNode == NULL) {
        return;
    }

    if (*head == NULL) {
        *head = newNode;
        return;
    }

    current = *head;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = newNode;
}

void printFileList(const FileNode* head) {
    const FileNode* current = head;

    printf("\n===== FileSoul 파일 목록 =====\n\n");

    while (current != NULL) {
        const FileSoul* file = &current->data;

        printf("파일명: %s\n", file->name);
        printf("경로: %s\n", file->path);
        printf("확장자: %s\n", file->extension[0] != '\0' ? file->extension : "(없음)");
        printf("크기: %lld bytes\n", file->size);
        printf("종류: %s\n", getFileTypeName(file->type));
        printf("기분: %s\n", getFileMoodName(file->mood));
        printf("성격: %s\n", getPersonalityName(file->personality));
        printf("관심도: %.2f\n", file->interest);
        printf("대사: %s\n", file->dialogue);
        printf("삭제 후보: %s\n", file->deleteCandidate ? "예" : "아니오");
        if (file->deleted || file->deleteFailed) {
            printf("삭제 결과: %s\n", file->deleteMessage);
        }
        printf("-----------------------------\n");

        current = current->next;
    }
}

void freeFileList(FileNode* head) {
    FileNode* current = head;
    FileNode* temp;

    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);
    }
}

void sortFileListByInterest(FileNode* head) {
    int swapped;
    FileNode* current;

    if (head == NULL) {
        return;
    }

    do {
        swapped = 0;
        current = head;

        while (current->next != NULL) {
            if (current->data.interest < current->next->data.interest) {
                FileSoul temp = current->data;
                current->data = current->next->data;
                current->next->data = temp;
                swapped = 1;
            }
            current = current->next;
        }
    } while (swapped);
}

void shuffleFileList(FileNode* head) {
    int count;
    int i;
    FileSoul* items;
    FileNode* current;

    count = countFileNodes(head);
    if (count <= 1) {
        return;
    }

    items = (FileSoul*)malloc(sizeof(FileSoul) * (size_t)count);
    if (items == NULL) {
        printf("추천 순서를 섞기 위한 메모리를 확보하지 못했습니다. 기존 순서를 사용합니다.\n");
        return;
    }

    current = head;
    for (i = 0; i < count && current != NULL; ++i) {
        items[i] = current->data;
        current = current->next;
    }

    for (i = count - 1; i > 0; --i) {
        int target = rand() % (i + 1);
        FileSoul temp = items[i];
        items[i] = items[target];
        items[target] = temp;
    }

    current = head;
    for (i = 0; i < count && current != NULL; ++i) {
        current->data = items[i];
        current = current->next;
    }

    free(items);
}

int countFileNodes(const FileNode* head) {
    int count = 0;

    while (head != NULL) {
        ++count;
        head = head->next;
    }

    return count;
}

const char* getChoiceName(UserChoice choice) {
    switch (choice) {
    case CHOICE_OPEN:
        return "열기";
    case CHOICE_KEEP:
        return "보관";
    case CHOICE_DELETE_CANDIDATE:
        return "삭제 후보 등록";
    case CHOICE_IGNORE:
        return "무시";
    case CHOICE_NONE:
    default:
        return "선택 없음";
    }
}
