#include <stdio.h>

#include "dialogue_view.h"
#include "personality.h"

void showPopupDialogues(FileNode* head) {
    FileNode* current = head;

    printf("\n===== FileSoul Dialogue =====\n\n");

    while (current != NULL) {
        printf("[%s]\n", current->name);
        printf("Type: %s\n", getFileTypeName(current->type));
        printf("Personality: %s\n", getPersonalityName(current->personality));
        printf("Dialogue: %s\n", current->dialogue);
        printf("Choice TODO: open / keep / delete candidate / ignore\n");
        printf("-----------------------------\n");

        current = current->next;
    }
}
