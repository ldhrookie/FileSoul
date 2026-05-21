#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_node.h"
#include "personality.h"

FileNode* createFileNode(const char* name, const char* extension, long long size) {
    FileNode* newNode = (FileNode*)malloc(sizeof(FileNode));

    if (newNode == NULL) {
        printf("메모리 할당 실패\n");
        return NULL;
    }

    strncpy(newNode->name, name, MAX_NAME_LENGTH - 1);
    newNode->name[MAX_NAME_LENGTH - 1] = '\0';

    strncpy(newNode->extension, extension, MAX_EXTENSION_LENGTH - 1);
    newNode->extension[MAX_EXTENSION_LENGTH - 1] = '\0';

    newNode->size = size;
    newNode->type = TYPE_UNKNOWN;
    newNode->personality = PERSONALITY_MYSTERIOUS;

    strcpy(newNode->dialogue, "아직 할 말이 정해지지 않았어.");

    newNode->next = NULL;

    return newNode;
}

void appendFileNode(FileNode** head, FileNode* newNode) {
    FileNode* current;

    if (newNode == NULL) {
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

void printFileList(FileNode* head) {
    FileNode* current = head;

    printf("===== FileSoul 파일 목록 =====\n\n");

    while (current != NULL) {
        printf("파일명: %s\n", current->name);
        printf("확장자: %s\n", current->extension);
        printf("크기: %lld bytes\n", current->size);
        printf("종류: %s\n", getFileTypeName(current->type));
        printf("성격: %s\n", getPersonalityName(current->personality));
        printf("대사: %s\n", current->dialogue);
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