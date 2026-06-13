#include "string_utils.h"

#include <ctype.h>
#include <stddef.h>

int equalsIgnoreCase(const char* left, const char* right) {
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
