#pragma once

char bspfilename[1024];

typedef float dvec_t;
typedef dvec_t dvec3_t[3];

#define PLANENUM_LEAF			-1

#define	MAX_MAP_HULLS			4

#define	MAX_MAP_MODELS		  256
#define	MAX_MAP_BRUSHES		 4096
#define	MAX_MAP_ENTITIES	 1024
#define	MAX_MAP_ENTSTRING	65536

#define	MAX_MAP_PLANES		8192
#define	MAX_MAP_NODES		32767		
#define	MAX_MAP_CLIPNODES	32767		
#define	MAX_MAP_LEAFS		32767		
#define	MAX_MAP_FACES		65535
#define	MAX_MAP_TEXINFO		4096 


#define MAX_PLANE_COUNT		32738
#define MAX_FACE_COUNT		65536
#define MAX_NODE_COUNT		65536
#define MAX_CLIPNODE_COUNT	65536
#define MAX_POINTS_ON_FACE	64
#define MAX_TEXTURE_INFO	65536

#define MAX_POINTS_ON_WINDING  64
#define MAX_FACES			  128


typedef enum {
	CONTENTS_EMPTY = -1,
	CONTENTS_SOLID = -2,
	CONTENTS_WATER = -3,
	CONTENTS_SLIME = -4,
	CONTENTS_LAVA  = -5,
	CONTENTS_SKY   = -6
} CONTENTS_CLASS;

typedef struct {
	float normal[3];
	float  dist;
} dplane_t;

typedef struct {

	unsigned short planenum;
	unsigned short texturenum;

	unsigned short numpoints;
	float* points;

} dface_t;

typedef struct {

	float mins[3];
	float maxs[3];

	short planenum;
	short children[2];

} dnode_t;

typedef struct {
	short planenum;
	short children[2];
} dclipnode_t;

typedef struct {
	int			contents;
	int			visofs;				

	float		mins[3];	
	float		maxs[3];

	unsigned short		firstface;
	unsigned short		numfaces;

} dleaf_t;

typedef struct {

	int texturenum;
	float vecs[2][4];
	int lightmapnum;

} texinfo_t;


int			numleafs;
dleaf_t		dleafs[MAX_MAP_LEAFS];

int			numplanes;
dplane_t	dplanes[MAX_MAP_PLANES];


int			numnodes;
dnode_t		dnodes[MAX_MAP_NODES];

int			numtexinfo;
texinfo_t	texinfo[MAX_MAP_TEXINFO];

int			numfaces;
dface_t		dfaces[MAX_MAP_FACES];

int			numclipnodes;
dclipnode_t	dclipnodes[MAX_MAP_CLIPNODES];






