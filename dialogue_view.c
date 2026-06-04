#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "dialogue_view.h"
#include "personality.h"
#include "stats.h"

static UserChoice readChoice(void) {
    char buffer[32];
    long value;
    char* end;

    printf("\n[1] 열기  [2] 보관  [3] 삭제 후보 등록  [4] 무시\n");
    printf("[5] 남은 파일 모두 무시  [6] 남은 파일 모두 삭제 후보  [0] 종료\n");
    printf("선택: ");

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("\n입력이 종료되었습니다. 남은 파일은 모두 무시됩니다.\n");
        return (UserChoice)5;
    }

    value = strtol(buffer, &end, 10);
    if (end == buffer) {
        return CHOICE_IGNORE;
    }

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

#ifdef _WIN32
static int utf8ToWide(const char* source, wchar_t* destination, int destinationSize) {
    int converted;

    if (destination == NULL || destinationSize <= 0) {
        return 0;
    }

    destination[0] = L'\0';
    if (source == NULL) {
        return 0;
    }

    converted = MultiByteToWideChar(CP_UTF8, 0, source, -1, destination, destinationSize);
    if (converted == 0) {
        converted = MultiByteToWideChar(CP_ACP, 0, source, -1, destination, destinationSize);
    }

    if (converted == 0) {
        destination[0] = L'\0';
        return 0;
    }

    return 1;
}
#endif

static void showFloatingDialogue(const FileSoul* file, const char* sizeText) {
#ifdef _WIN32
    char message[1536];
    wchar_t wideMessage[2048];
    wchar_t wideTitle[64];
    HMODULE user32;
    typedef int (WINAPI* MessageBoxWFunction)(HWND, LPCWSTR, LPCWSTR, UINT);
    union {
        FARPROC raw;
        MessageBoxWFunction messageBoxW;
    } popupFunction;

    if (file == NULL) {
        return;
    }

    snprintf(message, sizeof(message),
             "파일: %.120s\n"
             "종류: %.80s\n"
             "기분: %.80s\n"
             "성격: %.80s\n"
             "크기: %.40s\n"
             "관심도: %.1f\n\n"
             "\"%.240s\"\n\n"
             "팝업을 닫은 뒤 터미널에서 행동을 선택하세요.",
             file->name,
             getFileTypeName(file->type),
             getFileMoodName(file->mood),
             getPersonalityName(file->personality),
             sizeText != NULL ? sizeText : "",
             file->interest,
             file->dialogue);

    if (!utf8ToWide(message, wideMessage, (int)(sizeof(wideMessage) / sizeof(wideMessage[0]))) ||
        !utf8ToWide("FileSoul 대화", wideTitle, (int)(sizeof(wideTitle) / sizeof(wideTitle[0])))) {
        return;
    }

    user32 = LoadLibraryA("user32.dll");
    if (user32 == NULL) {
        return;
    }

    popupFunction.raw = GetProcAddress(user32, "MessageBoxW");
    if (popupFunction.raw != NULL) {
        popupFunction.messageBoxW(NULL, wideMessage, wideTitle, MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
    }

    FreeLibrary(user32);
#else
    (void)file;
    (void)sizeText;
#endif
}

void showPopupDialogues(FileNode* head) {
    showPopupDialoguesLimited(head, countFileNodes(head));
}

void showPopupDialoguesLimited(FileNode* head, int maxFiles) {
    FileNode* current = head;
    int shown = 0;

    printf("\n===== FileSoul 대화 =====\n");

    if (maxFiles <= 0) {
        maxFiles = 10;
    }

    while (current != NULL && shown < maxFiles) {
        FileSoul* file = &current->data;
        UserChoice choice;
        char sizeText[64];

        formatSize(file->size, sizeText, sizeof(sizeText));
        showFloatingDialogue(file, sizeText);

        printf("\n---\n");
        printf("파일: %s\n", file->name);
        printf("종류: %s | 기분: %s | 성격: %s\n",
               getFileTypeName(file->type),
               getFileMoodName(file->mood),
               getPersonalityName(file->personality));
        printf("크기: %s | 관심도: %.1f\n", sizeText, file->interest);
        printf("\"%s\"\n", file->dialogue);

        choice = readChoice();
        if ((int)choice == 5) {
            while (current != NULL) {
                applyChoice(&current->data, CHOICE_IGNORE);
                current = current->next;
            }
            printf("남은 파일을 모두 무시했습니다.\n");
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
            printf("대화를 종료했습니다. 남은 파일은 모두 무시됩니다.\n");
            return;
        }

        applyChoice(file, choice);
        printf("저장된 선택: %s\n", getChoiceName(file->choice));

        current = current->next;
        ++shown;
    }

    while (current != NULL) {
        applyChoice(&current->data, CHOICE_IGNORE);
        current = current->next;
    }

    if (shown >= maxFiles) {
        printf("\n대화 개수 제한에 도달했습니다. 남은 파일은 모두 무시됩니다.\n");
    }
}
