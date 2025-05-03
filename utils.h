#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Function prototypes
int numElements(const char *fileName, int linecnt);
int numLines(const char *filename);
int *readLine(const char *fileName, int linecnt, int n);
#endif
