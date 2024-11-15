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

extern unsigned int facecount, nodecount, brushcount, entitycount, leafcount, surfacecount, windingcount, portalcount;

extern int* planecount;

extern int calls;

void PrintMemory(void);
void Error(char* error, ...);


