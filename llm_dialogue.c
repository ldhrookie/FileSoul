#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#endif

#include "llm_dialogue.h"
#include "personality.h"
#include "stats.h"

#define LLM_REQUEST_CAPACITY 4096
#define LLM_RESPONSE_CAPACITY 32768

static char llmStatus[192] = "LLM 대사를 아직 요청하지 않았습니다.";

static void setStatus(const char* status) {
    if (status != NULL) {
        snprintf(llmStatus, sizeof(llmStatus), "%s", status);
    }
}

const char* getLlmDialogueStatus(void) {
    return llmStatus;
}

static int appendText(char* destination, size_t capacity, size_t* length, const char* text) {
    size_t textLength;

    if (destination == NULL || length == NULL || text == NULL) {
        return 0;
    }

    textLength = strlen(text);
    if (*length + textLength + 1 > capacity) {
        return 0;
    }

    memcpy(destination + *length, text, textLength);
    *length += textLength;
    destination[*length] = '\0';
    return 1;
}

static int appendJsonEscaped(char* destination, size_t capacity, size_t* length, const char* text) {
    const unsigned char* current = (const unsigned char*)text;
    char escaped[8];

    while (current != NULL && *current != '\0') {
        if (*current == '"' || *current == '\\') {
            escaped[0] = '\\';
            escaped[1] = (char)*current;
            escaped[2] = '\0';
            if (!appendText(destination, capacity, length, escaped)) {
                return 0;
            }
        } else if (*current == '\n' || *current == '\r' || *current == '\t') {
            const char* replacement = *current == '\n' ? "\\n" : (*current == '\r' ? "\\r" : "\\t");
            if (!appendText(destination, capacity, length, replacement)) {
                return 0;
            }
        } else if (*current < 0x20) {
            snprintf(escaped, sizeof(escaped), "\\u%04x", *current);
            if (!appendText(destination, capacity, length, escaped)) {
                return 0;
            }
        } else {
            escaped[0] = (char)*current;
            escaped[1] = '\0';
            if (!appendText(destination, capacity, length, escaped)) {
                return 0;
            }
        }
        ++current;
    }

    return 1;
}

static int appendUtf8(char* destination, size_t capacity, size_t* length, unsigned int codePoint) {
    unsigned char bytes[4];
    size_t count;

    if (codePoint <= 0x7f) {
        bytes[0] = (unsigned char)codePoint;
        count = 1;
    } else if (codePoint <= 0x7ff) {
        bytes[0] = (unsigned char)(0xc0 | (codePoint >> 6));
        bytes[1] = (unsigned char)(0x80 | (codePoint & 0x3f));
        count = 2;
    } else {
        bytes[0] = (unsigned char)(0xe0 | (codePoint >> 12));
        bytes[1] = (unsigned char)(0x80 | ((codePoint >> 6) & 0x3f));
        bytes[2] = (unsigned char)(0x80 | (codePoint & 0x3f));
        count = 3;
    }

    if (*length + count + 1 > capacity) {
        return 0;
    }

    memcpy(destination + *length, bytes, count);
    *length += count;
    destination[*length] = '\0';
    return 1;
}

static int hexValue(char value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    }
    if (value >= 'a' && value <= 'f') {
        return value - 'a' + 10;
    }
    if (value >= 'A' && value <= 'F') {
        return value - 'A' + 10;
    }
    return -1;
}

static int extractJsonStringAfter(const char* response, const char* anchor, const char* key,
                                  char* destination, size_t capacity) {
    const char* start;
    const char* keyPosition;
    const char* current;
    size_t length = 0;

    if (response == NULL || key == NULL || destination == NULL || capacity == 0) {
        return 0;
    }

    destination[0] = '\0';
    start = anchor != NULL ? strstr(response, anchor) : response;
    keyPosition = start != NULL ? strstr(start, key) : NULL;
    if (keyPosition == NULL) {
        return 0;
    }

    current = strchr(keyPosition + strlen(key), ':');
    if (current == NULL) {
        return 0;
    }
    ++current;
    while (*current == ' ' || *current == '\t' || *current == '\r' || *current == '\n') {
        ++current;
    }
    if (*current != '"') {
        return 0;
    }
    ++current;

    while (*current != '\0' && *current != '"') {
        if (*current == '\\') {
            ++current;
            if (*current == 'n' || *current == 'r' || *current == 't') {
                char replacement[2] = {*current == 't' ? ' ' : '\n', '\0'};
                if (!appendText(destination, capacity, &length, replacement)) {
                    return 0;
                }
            } else if (*current == 'u') {
                int a = hexValue(current[1]);
                int b = hexValue(current[2]);
                int c = hexValue(current[3]);
                int d = hexValue(current[4]);
                if (a < 0 || b < 0 || c < 0 || d < 0 ||
                    !appendUtf8(destination, capacity, &length,
                                (unsigned int)((a << 12) | (b << 8) | (c << 4) | d))) {
                    return 0;
                }
                current += 4;
            } else if (*current != '\0') {
                char literal[2] = {*current, '\0'};
                if (!appendText(destination, capacity, &length, literal)) {
                    return 0;
                }
            }
        } else {
            char literal[2] = {*current, '\0'};
            if (!appendText(destination, capacity, &length, literal)) {
                return 0;
            }
        }
        ++current;
    }

    return length > 0;
}

static int extractErrorMessage(const char* response, char* destination, size_t capacity) {
    if (extractJsonStringAfter(response, "\"error\"", "\"message\"", destination, capacity)) {
        return 1;
    }
    return extractJsonStringAfter(response, NULL, "\"message\"", destination, capacity);
}

static int extractOutputText(const char* response, char* destination, size_t capacity) {
    if (extractJsonStringAfter(response, "\"output_text\"", "\"text\"", destination, capacity)) {
        return 1;
    }

    return extractJsonStringAfter(response, NULL, "\"output_text\"", destination, capacity);
}

static int buildRequest(const FileSoul* file, const char* model, char* request, size_t capacity) {
    static const char* styleCues[] = {
        "짧고 당당하게",
        "살짝 장난스럽게",
        "차분하고 진지하게",
        "은근히 시적인 말투로",
        "친한 친구처럼",
        "조금 까칠하지만 밉지 않게",
        "솔직하고 현실적으로",
        "호기심을 자극하게"
    };
    char sizeText[64];
    char prompt[1800];
    size_t length = 0;
    unsigned int styleIndex = 0;
    const unsigned char* current = (const unsigned char*)file->name;

    formatSize(file->size, sizeText, sizeof(sizeText));
    while (*current != '\0') {
        styleIndex = styleIndex * 33U + *current;
        ++current;
    }
    styleIndex %= (unsigned int)(sizeof(styleCues) / sizeof(styleCues[0]));

    snprintf(prompt, sizeof(prompt),
             "너는 FileSoul이라는 파일 정리 프로그램 속 파일이다. "
             "사용자에게 직접 말하는 자연스러운 한국어 대사 한 줄만 써라. "
             "설명, 따옴표, 목록, 파일 경로는 쓰지 말고 90자 이내로 말해라. "
             "삭제를 강요하거나 안전 검사를 무시하게 하지 마라. "
             "같은 성격의 다른 파일도 똑같이 말하지 않도록 파일명과 상태에서 개성을 만들어라. "
             "이번 파일의 말투 힌트는 '%s'이다. "
             "파일명은 '%s', 종류는 '%s', 성격은 '%s', 현재 기분은 '%s', "
             "크기는 '%s', 정리 관심도는 %.1f/100이다. "
             "파일명과 상태를 재치 있게 활용하고, 특히 성격과 기분이 분명히 느껴지게 말해라.",
             styleCues[styleIndex],
             file->name,
             getFileTypeName(file->type),
             getPersonalityName(file->personality),
             getFileMoodName(file->mood),
             sizeText,
             file->interest);

    request[0] = '\0';
    return appendText(request, capacity, &length, "{\"model\":\"") &&
           appendJsonEscaped(request, capacity, &length, model) &&
           appendText(request, capacity, &length, "\",\"input\":\"") &&
           appendJsonEscaped(request, capacity, &length, prompt) &&
           appendText(request, capacity, &length, "\",\"max_output_tokens\":120}");
}

#ifdef _WIN32
typedef HINTERNET(WINAPI* WinHttpOpenFunction)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
typedef HINTERNET(WINAPI* WinHttpConnectFunction)(HINTERNET, LPCWSTR, INTERNET_PORT, DWORD);
typedef HINTERNET(WINAPI* WinHttpOpenRequestFunction)(HINTERNET, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR,
                                                      LPCWSTR const*, DWORD);
typedef BOOL(WINAPI* WinHttpSendRequestFunction)(HINTERNET, LPCWSTR, DWORD, LPVOID, DWORD, DWORD, DWORD_PTR);
typedef BOOL(WINAPI* WinHttpReceiveResponseFunction)(HINTERNET, LPVOID);
typedef BOOL(WINAPI* WinHttpQueryDataAvailableFunction)(HINTERNET, LPDWORD);
typedef BOOL(WINAPI* WinHttpReadDataFunction)(HINTERNET, LPVOID, DWORD, LPDWORD);
typedef BOOL(WINAPI* WinHttpCloseHandleFunction)(HINTERNET);
typedef BOOL(WINAPI* WinHttpSetTimeoutsFunction)(HINTERNET, int, int, int, int);
typedef BOOL(WINAPI* WinHttpQueryHeadersFunction)(HINTERNET, DWORD, LPCWSTR, LPVOID, LPDWORD, LPDWORD);

typedef struct {
    HMODULE library;
    WinHttpOpenFunction open;
    WinHttpConnectFunction connect;
    WinHttpOpenRequestFunction openRequest;
    WinHttpSendRequestFunction sendRequest;
    WinHttpReceiveResponseFunction receiveResponse;
    WinHttpQueryDataAvailableFunction queryDataAvailable;
    WinHttpReadDataFunction readData;
    WinHttpCloseHandleFunction closeHandle;
    WinHttpSetTimeoutsFunction setTimeouts;
    WinHttpQueryHeadersFunction queryHeaders;
} WinHttpApi;

static int loadWinHttp(WinHttpApi* api) {
    if (api == NULL) {
        return 0;
    }

    memset(api, 0, sizeof(*api));
    api->library = LoadLibraryA("winhttp.dll");
    if (api->library == NULL) {
        return 0;
    }

#define LOAD_WINHTTP(name, field)                                      \
    do {                                                               \
        FARPROC procedure = GetProcAddress(api->library, #name);       \
        memcpy(&api->field, &procedure, sizeof(api->field));           \
    } while (0)
    LOAD_WINHTTP(WinHttpOpen, open);
    LOAD_WINHTTP(WinHttpConnect, connect);
    LOAD_WINHTTP(WinHttpOpenRequest, openRequest);
    LOAD_WINHTTP(WinHttpSendRequest, sendRequest);
    LOAD_WINHTTP(WinHttpReceiveResponse, receiveResponse);
    LOAD_WINHTTP(WinHttpQueryDataAvailable, queryDataAvailable);
    LOAD_WINHTTP(WinHttpReadData, readData);
    LOAD_WINHTTP(WinHttpCloseHandle, closeHandle);
    LOAD_WINHTTP(WinHttpSetTimeouts, setTimeouts);
    LOAD_WINHTTP(WinHttpQueryHeaders, queryHeaders);
#undef LOAD_WINHTTP

    if (api->open == NULL || api->connect == NULL || api->openRequest == NULL ||
        api->sendRequest == NULL || api->receiveResponse == NULL ||
        api->queryDataAvailable == NULL || api->readData == NULL ||
        api->closeHandle == NULL || api->setTimeouts == NULL || api->queryHeaders == NULL) {
        FreeLibrary(api->library);
        memset(api, 0, sizeof(*api));
        return 0;
    }

    return 1;
}

static int requestDialogue(const char* apiKey, const char* body, char* response, size_t responseCapacity) {
    WinHttpApi api;
    HINTERNET session = NULL;
    HINTERNET connection = NULL;
    HINTERNET request = NULL;
    wchar_t headers[1024];
    size_t responseLength = 0;
    int success = 0;

    if (!loadWinHttp(&api)) {
        setStatus("winhttp.dll을 불러오지 못해 로컬 대사를 사용합니다.");
        return 0;
    }

    if (swprintf(headers, sizeof(headers) / sizeof(headers[0]),
                 L"Content-Type: application/json\r\nAuthorization: Bearer %hs\r\n", apiKey) < 0) {
        setStatus("LLM 요청 헤더를 만들지 못해 로컬 대사를 사용합니다.");
        goto cleanup;
    }

    session = api.open(L"FileSoul/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                       WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (session == NULL) {
        setStatus("LLM 연결 세션을 열지 못해 로컬 대사를 사용합니다.");
        goto cleanup;
    }
    api.setTimeouts(session, 5000, 5000, 15000, 30000);

    connection = api.connect(session, L"api.openai.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    request = connection != NULL
                  ? api.openRequest(connection, L"POST", L"/v1/responses", NULL,
                                    WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                    WINHTTP_FLAG_SECURE)
                  : NULL;
    if (request == NULL ||
        !api.sendRequest(request, headers, (DWORD)-1L, (LPVOID)body, (DWORD)strlen(body),
                         (DWORD)strlen(body), 0) ||
        !api.receiveResponse(request, NULL)) {
        setStatus("LLM API 요청에 실패해 로컬 대사를 사용합니다.");
        goto cleanup;
    }

    response[0] = '\0';
    while (responseLength + 1 < responseCapacity) {
        DWORD available = 0;
        DWORD read = 0;
        DWORD remaining;

        if (!api.queryDataAvailable(request, &available) || available == 0) {
            break;
        }
        remaining = (DWORD)(responseCapacity - responseLength - 1);
        if (available > remaining) {
            available = remaining;
        }
        if (!api.readData(request, response + responseLength, available, &read) || read == 0) {
            break;
        }
        responseLength += read;
        response[responseLength] = '\0';
    }

    success = responseLength > 0;
    if (!success) {
        setStatus("LLM API 응답이 비어 있어 로컬 대사를 사용합니다.");
    } else {
        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        DWORD index = 0;

        if (api.queryHeaders(request,
                             WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX,
                             &statusCode,
                             &statusCodeSize,
                             &index) &&
            (statusCode < 200 || statusCode >= 300)) {
            char errorMessage[256];

            if (extractErrorMessage(response, errorMessage, sizeof(errorMessage))) {
                snprintf(llmStatus, sizeof(llmStatus),
                         "LLM API HTTP %lu: %.120s",
                         (unsigned long)statusCode, errorMessage);
            } else {
                snprintf(llmStatus, sizeof(llmStatus),
                         "LLM API HTTP %lu 응답이라 로컬 대사를 사용합니다.",
                         (unsigned long)statusCode);
            }
            success = 0;
        }
    }

cleanup:
    if (request != NULL) {
        api.closeHandle(request);
    }
    if (connection != NULL) {
        api.closeHandle(connection);
    }
    if (session != NULL) {
        api.closeHandle(session);
    }
    FreeLibrary(api.library);
    return success;
}
#endif

int generateLlmDialogue(FileSoul* file) {
    const char* apiKey;
    const char* model;
    char request[LLM_REQUEST_CAPACITY];
    char response[LLM_RESPONSE_CAPACITY];
    char dialogue[MAX_DIALOGUE_LENGTH];

    if (file == NULL) {
        setStatus("LLM 대사를 만들 파일 정보가 없습니다.");
        return 0;
    }

    apiKey = getenv("OPENAI_API_KEY");
    if (apiKey == NULL || apiKey[0] == '\0') {
        setStatus("OPENAI_API_KEY가 없어 성격 기반 로컬 대사를 사용합니다.");
        return 0;
    }

    model = getenv("FILESOUL_LLM_MODEL");
    if (model == NULL || model[0] == '\0') {
        model = "gpt-4.1-mini";
    }

    if (!buildRequest(file, model, request, sizeof(request))) {
        setStatus("LLM 요청이 너무 길어 성격 기반 로컬 대사를 사용합니다.");
        return 0;
    }

#ifdef _WIN32
    if (!requestDialogue(apiKey, request, response, sizeof(response)) ||
        !extractOutputText(response, dialogue, sizeof(dialogue))) {
        if (strstr(getLlmDialogueStatus(), "로컬 대사") == NULL) {
            setStatus("LLM 응답에서 대사를 읽지 못해 로컬 대사를 사용합니다.");
        }
        return 0;
    }

    snprintf(file->dialogue, sizeof(file->dialogue), "%s", dialogue);
    setStatus("LLM이 파일 성격과 상태를 반영한 대사를 만들었습니다.");
    return 1;
#else
    (void)request;
    (void)response;
    (void)dialogue;
    setStatus("현재 운영체제에서는 LLM 연결을 지원하지 않아 로컬 대사를 사용합니다.");
    return 0;
#endif
}
