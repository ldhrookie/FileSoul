#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#endif

#include "dialogue_view.h"
#include "console_io.h"
#include "llm_dialogue.h"
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

static UserChoice showFloatingDialogue(const FileSoul* file, const char* sizeText) {
#ifdef _WIN32
    const char* terminalOnly = getenv("FILESOUL_TERMINAL_DIALOGUE");
    char message[1536];
    char title[320];
    wchar_t wideMessage[2048];
    wchar_t wideTitle[384];
    wchar_t openText[32];
    wchar_t keepText[32];
    wchar_t deleteText[32];
    wchar_t ignoreText[32];
    HMODULE comctl32;
    HMODULE user32;
    typedef HRESULT(WINAPI* TaskDialogIndirectFunction)(const TASKDIALOGCONFIG*, int*, int*, BOOL*);
    typedef int (WINAPI* MessageBoxWFunction)(HWND, LPCWSTR, LPCWSTR, UINT);
    union {
        FARPROC raw;
        TaskDialogIndirectFunction taskDialogIndirect;
    } taskDialogFunction;
    union {
        FARPROC raw;
        MessageBoxWFunction messageBoxW;
    } messageBoxFunction;
    TASKDIALOG_BUTTON buttons[4];
    TASKDIALOGCONFIG config;
    int selectedButton = 104;
    int fallbackResult;

    if (file == NULL) {
        return CHOICE_IGNORE;
    }
    if (terminalOnly != NULL && strcmp(terminalOnly, "1") == 0) {
        return CHOICE_NONE;
    }

    snprintf(message, sizeof(message),
             "\"%.240s\"\n\n"
             "종류: %.80s  |  기분: %.80s\n"
             "성격: %.80s  |  크기: %.40s\n"
             "정리 관심도: %.1f / 100",
             file->dialogue,
             getFileTypeName(file->type),
             getFileMoodName(file->mood),
             getPersonalityName(file->personality),
             sizeText != NULL ? sizeText : "",
             file->interest);
    snprintf(title, sizeof(title), "%.240s의 메시지", file->name);

    if (!utf8ToWide(message, wideMessage, (int)(sizeof(wideMessage) / sizeof(wideMessage[0]))) ||
        !utf8ToWide(title, wideTitle, (int)(sizeof(wideTitle) / sizeof(wideTitle[0]))) ||
        !utf8ToWide("열기", openText, (int)(sizeof(openText) / sizeof(openText[0]))) ||
        !utf8ToWide("보관", keepText, (int)(sizeof(keepText) / sizeof(keepText[0]))) ||
        !utf8ToWide("삭제 후보", deleteText, (int)(sizeof(deleteText) / sizeof(deleteText[0]))) ||
        !utf8ToWide("무시", ignoreText, (int)(sizeof(ignoreText) / sizeof(ignoreText[0])))) {
        return CHOICE_IGNORE;
    }

    buttons[0].nButtonID = 101;
    buttons[0].pszButtonText = openText;
    buttons[1].nButtonID = 102;
    buttons[1].pszButtonText = keepText;
    buttons[2].nButtonID = 103;
    buttons[2].pszButtonText = deleteText;
    buttons[3].nButtonID = 104;
    buttons[3].pszButtonText = ignoreText;

    memset(&config, 0, sizeof(config));
    config.cbSize = sizeof(config);
    config.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_SIZE_TO_CONTENT;
    config.pszWindowTitle = L"FileSoul";
    config.pszMainInstruction = wideTitle;
    config.pszContent = wideMessage;
    config.cButtons = (UINT)(sizeof(buttons) / sizeof(buttons[0]));
    config.pButtons = buttons;
    config.nDefaultButton = 102;

    comctl32 = LoadLibraryA("comctl32.dll");
    if (comctl32 != NULL) {
        taskDialogFunction.raw = GetProcAddress(comctl32, "TaskDialogIndirect");
        if (taskDialogFunction.raw != NULL &&
            SUCCEEDED(taskDialogFunction.taskDialogIndirect(&config, &selectedButton, NULL, NULL))) {
            FreeLibrary(comctl32);
            switch (selectedButton) {
            case 101:
                return CHOICE_OPEN;
            case 102:
                return CHOICE_KEEP;
            case 103:
                return CHOICE_DELETE_CANDIDATE;
            case 104:
            default:
                return CHOICE_IGNORE;
            }
        }
        FreeLibrary(comctl32);
    }

    user32 = LoadLibraryA("user32.dll");
    if (user32 != NULL) {
        messageBoxFunction.raw = GetProcAddress(user32, "MessageBoxW");
        if (messageBoxFunction.raw != NULL) {
            fallbackResult = messageBoxFunction.messageBoxW(
                NULL, wideMessage, wideTitle,
                MB_YESNOCANCEL | MB_ICONINFORMATION | MB_SETFOREGROUND);
            FreeLibrary(user32);
            if (fallbackResult == IDYES) {
                return CHOICE_KEEP;
            }
            if (fallbackResult == IDNO) {
                return CHOICE_DELETE_CANDIDATE;
            }
            return CHOICE_IGNORE;
        }
        FreeLibrary(user32);
    }
    return CHOICE_NONE;
#else
    (void)file;
    (void)sizeText;
    return CHOICE_NONE;
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
        generateLlmDialogue(file);
        choice = showFloatingDialogue(file, sizeText);

        printf("\n---\n");
        printf("파일: %s\n", file->name);
        printf("종류: %s | 기분: %s | 성격: %s\n",
               getFileTypeName(file->type),
               getFileMoodName(file->mood),
               getPersonalityName(file->personality));
        printf("크기: %s | 관심도: %.1f\n", sizeText, file->interest);
        printf("\"%s\"\n", file->dialogue);
        if (shown == 0) {
            printf("대사 생성 상태: %s\n", getLlmDialogueStatus());
        }

        if (choice == CHOICE_NONE) {
            choice = readChoice();
        }
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
