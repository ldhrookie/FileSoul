#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "personality.h"

static int extensionEquals(const char* left, const char* right) {
    if (left == NULL || right == NULL) {
        return 0;
    }

    while (*left != '\0' && *right != '\0') {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right)) {
            return 0;
        }
        ++left;
        ++right;
    }

    return *left == '\0' && *right == '\0';
}

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

FileType getFileType(const char* extension) {
    if (extension == NULL || extension[0] == '\0') {
        return TYPE_UNKNOWN;
    }

    if (extensionEquals(extension, "txt")) {
        return TYPE_TEXT;
    }

    if (extensionEquals(extension, "png") || extensionEquals(extension, "jpg") ||
        extensionEquals(extension, "jpeg") || extensionEquals(extension, "gif")) {
        return TYPE_IMAGE;
    }

    if (extensionEquals(extension, "c") || extensionEquals(extension, "h") ||
        extensionEquals(extension, "py") || extensionEquals(extension, "java")) {
        return TYPE_CODE;
    }

    if (extensionEquals(extension, "docx") || extensionEquals(extension, "pdf") ||
        extensionEquals(extension, "pptx")) {
        return TYPE_DOCUMENT;
    }

    if (extensionEquals(extension, "exe")) {
        return TYPE_EXECUTABLE;
    }

    if (extensionEquals(extension, "zip") || extensionEquals(extension, "rar") ||
        extensionEquals(extension, "7z")) {
        return TYPE_ARCHIVE;
    }

    return TYPE_UNKNOWN;
}

FileMood getFileMood(const FileSoul* file) {
    int days;

    if (file == NULL) {
        return MOOD_UNKNOWN;
    }

    days = daysSince(file->modifiedTime);

    if (file->interest >= 75.0) {
        return MOOD_URGENT;
    }

    if (file->size >= 50LL * 1024LL * 1024LL) {
        return MOOD_HEAVY;
    }

    if (days >= 180) {
        return MOOD_LONELY;
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
    if (file == NULL) {
        return;
    }

    switch (file->personality) {
    case PERSONALITY_DILIGENT:
        snprintf(file->dialogue, sizeof(file->dialogue),
                 "나는 아직 쓸모가 있어요. 열어보기 전에 함부로 정리하지 말아 주세요.");
        break;
    case PERSONALITY_LAZY:
        snprintf(file->dialogue, sizeof(file->dialogue),
                 "나는 조용히 쉬고 있었어요. 정말 필요한 파일인지 한 번만 확인해 주세요.");
        break;
    case PERSONALITY_HEAVY:
        snprintf(file->dialogue, sizeof(file->dialogue),
                 "나는 꽤 무거워요. 보관할 가치가 있는지 살펴봐 주세요.");
        break;
    case PERSONALITY_OLD:
        snprintf(file->dialogue, sizeof(file->dialogue),
                 "나는 오래 기다렸어요. 추억인지, 정리할 때가 된 건지 정해주세요.");
        break;
    case PERSONALITY_DANGEROUS:
        snprintf(file->dialogue, sizeof(file->dialogue),
                 "나는 조심해서 다뤄야 해요. 삭제하지는 말고 후보로만 표시해 주세요.");
        break;
    case PERSONALITY_MYSTERIOUS:
    default:
        snprintf(file->dialogue, sizeof(file->dialogue),
                 "나는 정체가 애매해요. 내가 왜 여기 있는지 확인해 볼래요?");
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
        return "평온함";
    case MOOD_LONELY:
        return "외로움";
    case MOOD_HEAVY:
        return "무거움";
    case MOOD_URGENT:
        return "급함";
    case MOOD_UNKNOWN:
    default:
        return "알 수 없음";
    }
}
