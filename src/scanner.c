#include "scanner.h"

#include <stdio.h>

int scanner_validate_folder(const char *folder_path)
{
    if (folder_path == NULL || folder_path[0] == '\0') {
        return 0;
    }

    return 1;
}

FileNode *scanner_scan_folder(const char *folder_path)
{
    if (!scanner_validate_folder(folder_path)) {
        fprintf(stderr, "잘못된 폴더 경로입니다.\n");
        return NULL;
    }

    printf("TODO: 파일 스캔 기능은 다음 단계에서 구현됩니다.\n");
    return NULL;
}

