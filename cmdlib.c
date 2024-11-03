#include "cmdlib.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <direct.h>
#endif

#ifdef NeXT
#include <libc.h>
#endif

unsigned int facecount, nodecount, brushcount, entitycount, leafcount, planecount, surfacecount, windingcount;

void PrintMemory(void) {
	printf("Windings: %u\n", windingcount);
	printf("Planes: %u\n", planecount);
	printf("Facecount: %u\n", facecount);
	printf("Brushes: %u\n", brushcount);
	printf("Entities: %u\n", entitycount);
	printf("Nodes: %u\n", nodecount);
	printf("Leafes: %u\n", leafcount);
	printf("Surfaces %u\n", surfacecount);
}

void Error(char* error, ...)
{
	va_list argptr;

	printf("************ ERROR ************\n");

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}