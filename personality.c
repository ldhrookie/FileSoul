#include <stdio.h>
#include <string.h>
#include <time.h>

#include "personality.h"
#include "string_utils.h"

static int daysSince(time_t value) {
    time_t now;

    if (value <= 0) {
        return 9999;
    }

    now = time(NULL);
    if (now <= value) {
        return 0;
    }

    return (int)((now - value) / (60 * 60 * 24));
}

static unsigned int dialogueSeed(const FileSoul* file) {
    const unsigned char* current;
    unsigned int hash = 2166136261U;

    if (file == NULL) {
        return 0;
    }

    current = (const unsigned char*)file->name;
    while (*current != '\0') {
        hash = (hash ^ *current) * 16777619U;
        ++current;
    }

    current = (const unsigned char*)file->extension;
    while (*current != '\0') {
        hash = (hash ^ *current) * 16777619U;
        ++current;
    }

    hash ^= (unsigned int)file->size;
    hash ^= (unsigned int)(file->size >> 32);
    hash ^= (unsigned int)file->modifiedTime;
    return hash;
}

static const char* getSizeAttitude(long long size) {
    if (size >= 50LL * 1024LL * 1024LL) {
        return "자리 차지는 크지만 존재감도 확실해";
    }
    if (size >= 1024LL * 1024LL) {
        return "조금 묵직하지만 감당 못 할 정도는 아니야";
    }
    if (size == 0) {
        return "몸집은 비어 있어도 사연은 있을지 몰라";
    }
    return "자리도 거의 안 차지하는 편이야";
}

FileType getFileType(const char* extension) {
    if (extension == NULL || extension[0] == '\0') {
        return TYPE_UNKNOWN;
    }

    if (equalsIgnoreCase(extension, "txt")) {
        return TYPE_TEXT;
    }

    if (equalsIgnoreCase(extension, "png") || equalsIgnoreCase(extension, "jpg") ||
        equalsIgnoreCase(extension, "jpeg") || equalsIgnoreCase(extension, "gif") ||
        equalsIgnoreCase(extension, "bmp")) {
        return TYPE_IMAGE;
    }

    if (equalsIgnoreCase(extension, "c") || equalsIgnoreCase(extension, "h") ||
        equalsIgnoreCase(extension, "cpp") || equalsIgnoreCase(extension, "py") ||
        equalsIgnoreCase(extension, "java") || equalsIgnoreCase(extension, "js")) {
        return TYPE_CODE;
    }

    if (equalsIgnoreCase(extension, "docx") || equalsIgnoreCase(extension, "pdf") ||
        equalsIgnoreCase(extension, "pptx") || equalsIgnoreCase(extension, "hwp")) {
        return TYPE_DOCUMENT;
    }

    if (equalsIgnoreCase(extension, "exe") || equalsIgnoreCase(extension, "dll") ||
        equalsIgnoreCase(extension, "bat") || equalsIgnoreCase(extension, "cmd")) {
        return TYPE_EXECUTABLE;
    }

    if (equalsIgnoreCase(extension, "zip") || equalsIgnoreCase(extension, "rar") ||
        equalsIgnoreCase(extension, "7z") || equalsIgnoreCase(extension, "tar") ||
        equalsIgnoreCase(extension, "gz")) {
        return TYPE_ARCHIVE;
    }

    return TYPE_UNKNOWN;
}

FileMood getFileMood(const FileSoul* file) {
    int days;
    unsigned int seed;

    if (file == NULL) {
        return MOOD_UNKNOWN;
    }

    days = daysSince(file->modifiedTime);
    seed = dialogueSeed(file);

    if (file->type == TYPE_EXECUTABLE || file->type == TYPE_UNKNOWN) {
        return MOOD_URGENT;
    }

    if (file->size >= 50LL * 1024LL * 1024LL) {
        return MOOD_HEAVY;
    }

    if (file->interest >= 85.0) {
        return MOOD_URGENT;
    }

    if (days >= 365) {
        switch (seed % 4U) {
        case 0:
            return MOOD_LONELY;
        case 1:
            return MOOD_CURIOUS;
        case 2:
            return MOOD_HEAVY;
        default:
            return MOOD_CALM;
        }
    }

    if (days >= 180) {
        switch (seed % 4U) {
        case 0:
            return MOOD_LONELY;
        case 1:
            return MOOD_CURIOUS;
        case 2:
            return MOOD_LIVELY;
        default:
            return MOOD_CALM;
        }
    }

    if (file->type == TYPE_IMAGE || file->type == TYPE_ARCHIVE) {
        return (seed % 3U) == 0U ? MOOD_LIVELY : MOOD_CURIOUS;
    }

    if (file->interest >= 65.0) {
        return MOOD_URGENT;
    }

    return MOOD_CALM;
}

Personality getPersonality(FileType type, long long size, time_t modifiedTime) {
    int days = daysSince(modifiedTime);

    if (type == TYPE_EXECUTABLE || type == TYPE_UNKNOWN) {
        return PERSONALITY_DANGEROUS;
    }

    if (size >= 50LL * 1024LL * 1024LL) {
        return PERSONALITY_HEAVY;
    }

    if (days >= 180) {
        return PERSONALITY_OLD;
    }

    if (type == TYPE_CODE || type == TYPE_DOCUMENT) {
        return PERSONALITY_DILIGENT;
    }

    if (type == TYPE_TEXT) {
        return PERSONALITY_LAZY;
    }

    return PERSONALITY_MYSTERIOUS;
}

double calculateBasicInterest(const FileSoul* file) {
    double score = 10.0;
    int days;

    if (file == NULL) {
        return 0.0;
    }

    days = daysSince(file->modifiedTime);

    if (file->size > 0) {
        score += (double)(file->size / (1024LL * 1024LL)) * 3.0;
    }

    if (days > 365) {
        score += 25.0;
    } else if (days > 180) {
        score += 15.0;
    } else if (days > 30) {
        score += 5.0;
    }

    if (file->type == TYPE_EXECUTABLE || file->type == TYPE_UNKNOWN) {
        score += 20.0;
    }

    if (file->type == TYPE_DOCUMENT || file->type == TYPE_CODE) {
        score -= 5.0;
    }

    if (score < 0.0) {
        score = 0.0;
    }
    if (score > 100.0) {
        score = 100.0;
    }

    return score;
}

void generateDialogue(FileSoul* file) {
    unsigned int variant;
    int days;

    if (file == NULL) {
        return;
    }

    variant = dialogueSeed(file) % 4U;
    days = daysSince(file->modifiedTime);

    switch (file->personality) {
    case PERSONALITY_DILIGENT:
        if (variant == 0) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 할 일은 끝까지 해내는 %s이야. 아직 맡길 일이 남았는지 확인해 줘.",
                     file->name, getFileTypeName(file->type));
        } else if (variant == 1) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 수정된 지 %d일 됐어. 기록을 정리해 두는 건 내 전문이지.",
                     file->name, days);
        } else if (variant == 2) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: %s답게 차근차근 준비돼 있어. 보관할 가치가 있는지 봐 줘.",
                     file->name, getFileTypeName(file->type));
        } else {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 난 성실하게 자리를 지켰어. 마지막 검토 없이 넘기지는 말아 줘.",
                     file->name);
        }
        break;
    case PERSONALITY_LAZY:
        if (variant == 0) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 난 조용히 쉬는 중이야. 급한 일 아니면 조금만 더 누워 있을게.",
                     file->name);
        } else if (variant == 1) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: %d일 동안 별일 없었네. 정리할 거면 살살 결정해 줘.",
                     file->name, days);
        } else if (variant == 2) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: %s. 굳이 나까지 바쁘게 움직여야 할까?",
                     file->name, getSizeAttitude(file->size));
        } else {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 내용은 텍스트지만 오늘은 말수가 적고 싶어. 보관 쪽이 편하긴 해.",
                     file->name);
        }
        break;
    case PERSONALITY_HEAVY:
        if (variant == 0) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 인정할게, 내가 공간을 꽤 차지해. 대신 쉽게 버릴 무게는 아닐걸.",
                     file->name);
        } else if (variant == 1) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: %s. 내 크기만 보고 성급히 판단하지는 마.",
                     file->name, getSizeAttitude(file->size));
        } else if (variant == 2) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 무거운 건 죄가 아니잖아. 내가 담고 있는 가치부터 확인해 줘.",
                     file->name);
        } else {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 정리 관심도 %.0f점이라니, 오늘 대화의 중심은 나인가 보네.",
                     file->name, file->interest);
        }
        break;
    case PERSONALITY_OLD:
        if (variant == 0) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: %d일을 기다렸어. 오래됐다는 이유만으로 내 이야기를 끝내진 말아 줘.",
                     file->name, days);
        } else if (variant == 1) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 한동안 날 찾지 않았지. 추억인지 짐인지 이번엔 정해 줘.",
                     file->name);
        } else if (variant == 2) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 시간은 많이 흘렀지만 나는 그대로야. 한 번쯤 열어 봐도 좋겠어.",
                     file->name);
        } else {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 오래된 %s에게도 마지막으로 할 말은 있는 법이야.",
                     file->name, getFileTypeName(file->type));
        }
        break;
    case PERSONALITY_DANGEROUS:
        if (variant == 0) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 경계하는 건 이해해. 그래도 내 정체를 확인한 다음 결정해 줘.",
                     file->name);
        } else if (variant == 1) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 나는 %s이라 조심해서 다뤄야 해. 보호 검사는 꼭 거쳐.",
                     file->name, getFileTypeName(file->type));
        } else if (variant == 2) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 확장자 .%s만 보고 수상하다고 단정한 건 아니지? 먼저 확인해 봐.",
                     file->name, file->extension[0] != '\0' ? file->extension : "(없음)");
        } else {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 정리 관심도 %.0f점이군. 날 건드릴 땐 안전 절차부터 챙겨.",
                     file->name, file->interest);
        }
        break;
    case PERSONALITY_MYSTERIOUS:
    default:
        if (variant == 0) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 내 정체가 바로 보이지 않는다고? 그게 내 매력일 수도 있지.",
                     file->name);
        } else if (variant == 1) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 나는 %s인데도 조금 수수께끼 같아. 열어 보면 답이 나올까?",
                     file->name, getFileTypeName(file->type));
        } else if (variant == 2) {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: %s. 작고 조용한 비밀일지도 몰라.",
                     file->name, getSizeAttitude(file->size));
        } else {
            snprintf(file->dialogue, sizeof(file->dialogue),
                     "%.70s: 이름만으로 나를 다 안다고 생각하지 마. 한 번 더 살펴봐 줘.",
                     file->name);
        }
        break;
    }
}

void assignPersonality(FileSoul* file, InterestCalculator calculator) {
    if (file == NULL) {
        return;
    }

    file->type = getFileType(file->extension);
    file->personality = getPersonality(file->type, file->size, file->modifiedTime);
    file->interest = calculator != NULL ? calculator(file) : calculateBasicInterest(file);
    file->mood = getFileMood(file);
    generateDialogue(file);

    switch (file->type) {
    case TYPE_CODE:
        file->extra.codeLineCount = (int)(file->size / 80);
        break;
    case TYPE_IMAGE:
        file->extra.imagePixelScore = (int)(file->size / 1024);
        break;
    case TYPE_DOCUMENT:
        file->extra.documentPageGuess = (int)(file->size / 3000) + 1;
        break;
    default:
        file->extra.extraScore = file->interest;
        break;
    }
}

void assignPersonalities(FileNode* head) {
    FileNode* current = head;

    while (current != NULL) {
        assignPersonality(&current->data, calculateBasicInterest);
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
    case TYPE_ARCHIVE:
        return "압축 파일";
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
        return "느긋한 파일";
    case PERSONALITY_HEAVY:
        return "무거운 파일";
    case PERSONALITY_OLD:
        return "오래 기다린 파일";
    case PERSONALITY_DANGEROUS:
        return "주의가 필요한 파일";
    case PERSONALITY_MYSTERIOUS:
    default:
        return "미스터리한 파일";
    }
}

const char* getFileMoodName(FileMood mood) {
    switch (mood) {
    case MOOD_CALM:
        return "편안함";
    case MOOD_LONELY:
        return "쓸쓸함";
    case MOOD_HEAVY:
        return "버거움";
    case MOOD_URGENT:
        return "불안함";
    case MOOD_CURIOUS:
        return "궁금함";
    case MOOD_LIVELY:
        return "들뜸";
    case MOOD_UNKNOWN:
    default:
        return "알 수 없음";
    }
}
