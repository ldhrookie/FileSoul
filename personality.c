#include <stdio.h>
#include <string.h>
#include "personality.h"

FileType getFileType(const char* extension) {
    if (strcmp(extension, "txt") == 0) {
        return TYPE_TEXT;
    }
    else if (strcmp(extension, "png") == 0 || strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0) {
        return TYPE_IMAGE;
    }
    else if (strcmp(extension, "c") == 0 || strcmp(extension, "h") == 0 || strcmp(extension, "py") == 0 || strcmp(extension, "java") == 0) {
        return TYPE_CODE;
    }
    else if (strcmp(extension, "docx") == 0 || strcmp(extension, "pdf") == 0 || strcmp(extension, "pptx") == 0) {
        return TYPE_DOCUMENT;
    }
    else if (strcmp(extension, "exe") == 0) {
        return TYPE_EXECUTABLE;
    }
    else {
        return TYPE_UNKNOWN;
    }
}

Personality getPersonality(FileType type, long long size) {
    if (type == TYPE_EXECUTABLE) {
        return PERSONALITY_DANGEROUS;
    }

    if (size >= 5000000) {
        return PERSONALITY_HEAVY;
    }

    if (type == TYPE_CODE) {
        return PERSONALITY_DILIGENT;
    }

    if (type == TYPE_IMAGE) {
        return PERSONALITY_OLD;
    }

    if (type == TYPE_DOCUMENT) {
        return PERSONALITY_DILIGENT;
    }

    if (type == TYPE_TEXT) {
        return PERSONALITY_LAZY;
    }

    return PERSONALITY_MYSTERIOUS;
}

void generateDialogue(FileNode* file) {
    if (file == NULL) {
        return;
    }

    switch (file->personality) {
    case PERSONALITY_DILIGENT:
        strcpy(file->dialogue, "나는 아직 쓸모 있어. 나를 열어보면 꽤 중요한 내용이 있을지도 몰라.");
        break;

    case PERSONALITY_LAZY:
        strcpy(file->dialogue, "나는 조용히 구석에 있었을 뿐이야. 지우기 전에 한 번만 확인해줘.");
        break;

    case PERSONALITY_HEAVY:
        strcpy(file->dialogue, "나 좀 무겁긴 해... 그래도 이유 없이 큰 건 아닐 수도 있어.");
        break;

    case PERSONALITY_OLD:
        strcpy(file->dialogue, "나는 추억을 담고 있어. 지우기 전에 네가 웃었던 순간을 떠올려봐.");
        break;

    case PERSONALITY_DANGEROUS:
        strcpy(file->dialogue, "나는 실행 파일이야. 정체를 모르면 함부로 실행하지 않는 게 좋아.");
        break;

    case PERSONALITY_MYSTERIOUS:
    default:
        strcpy(file->dialogue, "나는 정체를 알 수 없는 파일이야. 내가 왜 여기 있는지 너도 궁금하지 않아?");
        break;
    }
}

void assignPersonalities(FileNode* head) {
    FileNode* current = head;

    while (current != NULL) {
        current->type = getFileType(current->extension);
        current->personality = getPersonality(current->type, current->size);
        generateDialogue(current);

        current = current->next;
    }
}

const char* getFileTypeName(FileType type) {
    switch (type) {
    case TYPE_TEXT:
        return "텍스트 파일";

    case TYPE_IMAGE:
        return "이미지 파일";

    case TYPE_CODE:
        return "코드 파일";

    case TYPE_DOCUMENT:
        return "문서 파일";

    case TYPE_EXECUTABLE:
        return "실행 파일";

    case TYPE_UNKNOWN:
    default:
        return "알 수 없는 파일";
    }
}

const char* getPersonalityName(Personality personality) {
    switch (personality) {
    case PERSONALITY_DILIGENT:
        return "성실한 파일";

    case PERSONALITY_LAZY:
        return "게으른 파일";

    case PERSONALITY_HEAVY:
        return "무거운 파일";

    case PERSONALITY_OLD:
        return "추억에 잠긴 파일";

    case PERSONALITY_DANGEROUS:
        return "수상한 파일";

    case PERSONALITY_MYSTERIOUS:
    default:
        return "미스터리한 파일";
    }
}