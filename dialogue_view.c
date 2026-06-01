#include <stdio.h>
#include <stdlib.h>

#include "dialogue_view.h"
#include "personality.h"

static UserChoice readChoice(void) {
    char buffer[32];
    long value;
    char* end;

    printf("[1] 열기\n");
    printf("[2] 보관\n");
    printf("[3] 삭제 후보 등록\n");
    printf("[4] 무시\n");
    printf("[5] 남은 파일 모두 무시\n");
    printf("[6] 남은 파일 모두 삭제 후보 등록\n");
    printf("[0] 대화 종료\n");
    printf("선택: ");

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("\n입력이 종료되어 이 파일은 무시로 처리합니다.\n");
        return CHOICE_IGNORE;
    }

    value = strtol(buffer, &end, 10);
    (void)end;

    switch (value) {
    case 1:
        return CHOICE_OPEN;
    case 2:
        return CHOICE_KEEP;
    case 3:
        return CHOICE_DELETE_CANDIDATE;
    case 4:
        return CHOICE_IGNORE;
    case 5:
        return (UserChoice)5;
    case 6:
        return (UserChoice)6;
    case 0:
        return (UserChoice)0;
    default:
        return CHOICE_IGNORE;
    }
}

static void applyChoice(FileSoul* file, UserChoice choice) {
    if (file == NULL) {
        return;
    }

    file->choice = choice;
    file->deleteCandidate = choice == CHOICE_DELETE_CANDIDATE;
}

void showPopupDialogues(FileNode* head) {
    showPopupDialoguesLimited(head, countFileNodes(head));
}

void showPopupDialoguesLimited(FileNode* head, int maxFiles) {
    FileNode* current = head;
    int shown = 0;

    printf("\n===== FileSoul 대화 =====\n\n");

    if (maxFiles <= 0) {
        maxFiles = 10;
    }

    while (current != NULL && shown < maxFiles) {
        FileSoul* file = &current->data;
        UserChoice choice;

        printf("[%s]\n", file->name);
        printf("종류: %s\n", getFileTypeName(file->type));
        printf("기분: %s\n", getFileMoodName(file->mood));
        printf("성격: %s\n", getPersonalityName(file->personality));
        printf("관심 지수: %.2f\n", file->interest);
        printf("대사: %s\n", file->dialogue);

        choice = readChoice();
        if ((int)choice == 5) {
            while (current != NULL) {
                applyChoice(&current->data, CHOICE_IGNORE);
                current = current->next;
            }
            printf("남은 파일을 모두 무시로 처리했습니다.\n");
            return;
        }

        if ((int)choice == 6) {
            while (current != NULL) {
                applyChoice(&current->data, CHOICE_DELETE_CANDIDATE);
                current = current->next;
            }
            printf("남은 파일을 모두 삭제 후보로 등록했습니다.\n");
            return;
        }

        if ((int)choice == 0) {
            while (current != NULL) {
                applyChoice(&current->data, CHOICE_IGNORE);
                current = current->next;
            }
            printf("대화를 종료하고 남은 파일을 무시로 처리했습니다.\n");
            return;
        }

        applyChoice(file, choice);
        printf("기록된 선택: %s\n", getChoiceName(file->choice));
        printf("-----------------------------\n");

        current = current->next;
        ++shown;
    }

    while (current != NULL) {
        applyChoice(&current->data, CHOICE_IGNORE);
        current = current->next;
    }
}
