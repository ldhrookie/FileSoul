#ifndef CONSOLE_IO_H
#define CONSOLE_IO_H

int consolePrintf(const char* format, ...);

#ifndef CONSOLE_IO_IMPLEMENTATION
#define printf consolePrintf
#endif

#endif
