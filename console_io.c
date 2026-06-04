#define CONSOLE_IO_IMPLEMENTATION

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "console_io.h"

static int writeFormattedUtf8(const char* text, int length) {
#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;

    if (output != INVALID_HANDLE_VALUE && output != NULL && GetConsoleMode(output, &mode)) {
        int wideLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, length, NULL, 0);

        if (wideLength > 0) {
            wchar_t* wideText = (wchar_t*)malloc(((size_t)wideLength + 1U) * sizeof(wchar_t));
            if (wideText != NULL) {
                DWORD written = 0;

                MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, length, wideText, wideLength);
                wideText[wideLength] = L'\0';
                if (WriteConsoleW(output, wideText, (DWORD)wideLength, &written, NULL)) {
                    free(wideText);
                    return (int)written;
                }
                free(wideText);
            }
        }
    }
#endif

    return (int)fwrite(text, 1U, (size_t)length, stdout);
}

int consolePrintf(const char* format, ...) {
    va_list arguments;
    va_list copy;
    char stackBuffer[4096];
    char* output = stackBuffer;
    int required;
    int result;

    if (format == NULL) {
        return -1;
    }

    va_start(arguments, format);
    va_copy(copy, arguments);
    required = vsnprintf(stackBuffer, sizeof(stackBuffer), format, arguments);
    va_end(arguments);

    if (required < 0) {
        va_end(copy);
        return required;
    }

    if ((size_t)required >= sizeof(stackBuffer)) {
        output = (char*)malloc((size_t)required + 1U);
        if (output == NULL) {
            va_end(copy);
            return -1;
        }
        vsnprintf(output, (size_t)required + 1U, format, copy);
    }
    va_end(copy);

    result = writeFormattedUtf8(output, required);
    fflush(stdout);
    if (output != stackBuffer) {
        free(output);
    }
    return result;
}
