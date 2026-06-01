#ifndef SCANNER_H
#define SCANNER_H

#include "file_node.h"

int scanDirectory(const char* folderPath, FileNode** head);
void extractExtension(const char* filename, char* extension, int extensionSize);

#endif
