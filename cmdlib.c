#include "cmdlib.h"

unsigned int facecount, nodecount, brushcount, entitycount;
unsigned int planecount, leafcount;

void PrintMemory(void) {
	printf("Planes: %u\n", planecount);
	printf("Facecount: %u\n", facecount);
	printf("Brushes: %u\n", brushcount);
	printf("Entities: %u\n", entitycount);
	printf("Nodes: %u\n", nodecount);
	printf("Leafes: %u\n", leafcount);
}