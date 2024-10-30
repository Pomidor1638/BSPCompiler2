#pragma once

typedef float dvec_t;
typedef dvec_t dvec3_t[3];

#define PLANENUM_LEAF -1


#define MAX_MAP_PLANES 65536


#define MAX_PLANE_COUNT 32738
#define MAX_FACE_COUNT 65536
#define MAX_NODE_COUNT 65536
#define MAX_CLIPNODE_COUNT 65536
#define MAX_POINTS_ON_FACE 64

#define MAX_POINTS_ON_WINDING 64
#define MAX_FACES 128


typedef enum {
	CONTENTS_EMPTY = -1,
	CONTENTS_SOLID = -2,
	CONTENTS_WATER = -3,
	CONTENTS_SLIME = -4,
	CONTENTS_LAVA  = -5,
	CONTENTS_SKY   = -6
} CONTENTS_CLASS;

typedef struct {
	dvec3_t normal;
	dvec_t  dist;
} dplane_t;

typedef struct {

	unsigned short planenum;
	unsigned short texturenum;
	unsigned short pointsnum;

	dvec3_t* points;

} dface_t;

typedef struct {

	dvec3_t mins, maxs;

	short planenum;
	short children[2];

} dnode_t;

typedef struct {
	short planenum;
	short children[2];
} dclipnode_t;

typedef struct {
	dvec3_t mins, maxs;

	unsigned int first_face;
	unsigned short facenum;

	short visinfo;
} dleaf_t;



extern int dplanes_count;
extern dplane_t dplanes[MAX_PLANE_COUNT];

extern int dfaces_count;
extern dface_t dfaces[MAX_FACE_COUNT];

extern int dnodes_count;
extern dnode_t dnodes[MAX_NODE_COUNT];

extern int dclipnodes_count;
extern dclipnode_t dclipnodes[MAX_FACE_COUNT];





