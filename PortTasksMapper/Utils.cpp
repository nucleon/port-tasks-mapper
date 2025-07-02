#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include "Utils.h"

unsigned char* loadFileBytes(const char* filename, size_t* outSize)
{
	FILE* file = fopen(filename, "rb");
	if (!file) return nullptr;

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);

	unsigned char* buffer = (unsigned char*)malloc(size);
	if (!buffer) { fclose(file); return nullptr; }

	fread(buffer, 1, size, file);
	fclose(file);

	*outSize = size;
	return buffer;
}
