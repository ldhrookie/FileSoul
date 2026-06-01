#ifndef SCANNER_H
#define SCANNER_H

#include "file_soul.h"

int scanner_validate_folder(const char *folder_path);
FileNode *scanner_scan_folder(const char *folder_path);

#endif

