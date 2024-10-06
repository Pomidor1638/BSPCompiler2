#pragma once

typedef float dvec_t;
typedef dvec_t dvec3_t[3];

#define MAX_POINTS_ON_FACE 64

typedef enum {
	CONTENTS_EMPTY = -1,
	CONTENTS_SOLID = -2,
	CONTENTS_EMPTY = -3,
};

typedef struct {
	dvec3_t normal;
	dvec_t dist;
} dplane_t;

typedef struct {
	unsigned short planenum;
	unsigned short texturenum;
	unsigned short pointsnum;
	dvec3_t points[MAX_POINTS_ON_FACE];
} dface_t;