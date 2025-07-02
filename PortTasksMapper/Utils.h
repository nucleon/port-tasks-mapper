#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <stddef.h>

unsigned char* loadFileBytes(const char* filename, size_t* outSize);

#endif // FILEUTILS_H
