#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <memory.h>


typedef unsigned char byte;

extern unsigned int facecount, nodecount, brushcount, entitycount, leafcount, planecount, surfacecount, windingcount;

extern void PrintMemory(void);
extern void Error(char* error, ...);

