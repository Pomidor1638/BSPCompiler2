#include "cmdlib.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <direct.h>
#endif

#ifdef NeXT
#include <libc.h>
#endif

unsigned int facecount, nodecount, brushcount, entitycount, leafcount, surfacecount, windingcount, portalcount;

int* planecount;

int calls;

void PrintMemory(void) {
	printf("%6u windings\n", windingcount);
	printf("%6u planes\n",    *planecount);
	printf("%6u facecount\n",   facecount);
	printf("%6u brushes\n",	   brushcount);
	printf("%6u entities\n",  entitycount);
	printf("%6u nodes\n",	    nodecount);
	printf("%6u leafes\n",	    leafcount);
	printf("%6u surfaces\n", surfacecount);
	printf("%6u portals\n",   portalcount);
	printf("%6u calls\n", calls);
}

void Error(char* error, ...) {

	va_list argptr;

	printf("************ ERROR ************\n");

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}