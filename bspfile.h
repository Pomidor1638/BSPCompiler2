#pragma once
#include <stdio.h>
#include <memory.h>

typedef unsigned char byte;

typedef float dvec_t;
typedef dvec_t dvec3_t[3];

#define PLANENUM_LEAF			-1

#define	MAX_MAP_HULLS		4

#define	MAX_MAP_MODELS		255
#define	MAX_MAP_BRUSHES		4095
#define	MAX_MAP_ENTITIES	1023
#define	MAX_MAP_ENTSTRING	65535

#define	MAX_MAP_PLANES		8192
#define	MAX_MAP_NODES		8192
#define	MAX_MAP_CLIPNODES	8192
#define	MAX_MAP_LEAFS		8192	
#define	MAX_MAP_VERTS		32768
#define	MAX_MAP_VERTEXTABLE 65535*2
#define	MAX_MAP_FACES		32768	
#define MAX_MAP_PORTALS     32768
#define MAX_MAP_PORTALTABLE 32768
#define	MAX_MAP_TEXINFO		4095
#define	MAX_MAP_LIGHTING	0x100000

#define MAX_POINTS_ON_WINDING  64
#define MAX_FACES			  128

#define	MAX_KEY		32
#define	MAX_VALUE	1024


#define BSPVERSION	130



enum LUMP_SHIT {
	LUMP_PLANES,
	LUMP_VERTEXES,
	LUMP_VTABLE,
	LUMP_PORTALS,
	LUMP_PORTALTABLE,
	LUMP_FACES,
	LUMP_NODES,
	LUMP_LEAFS,
	LUMP_CLIPNODES,
	LUMP_TEXINFO,
	LUMP_LIGHTING,
	LUMP_ENTITIES,
	LUMP_MODELS,
};

#define	HEADER_LUMPS	  13


typedef struct {
	float		mins[3], maxs[3];
	float		origin[3];
	int			headnode[MAX_MAP_HULLS];
	int			visleafs;
	int			firstface, numfaces;
} dmodel_t;

typedef struct {
	int		fileofs, filelen;
} lump_t;

typedef struct {
	int			version;
	lump_t		lumps[HEADER_LUMPS];
} dheader_t;

typedef enum {
	CONTENTS_EMPTY = -1,
	CONTENTS_SOLID = -2,
	CONTENTS_WATER = -3,
	CONTENTS_SLIME = -4,
	CONTENTS_LAVA  = -5,
	CONTENTS_SKY   = -6,
} CONTENTS_CLASS;

typedef struct {
	float normal[3];
	float  dist;
} dplane_t;

typedef struct {
	float v[3];
} dvertex_t;

typedef struct {

	short planenum;
	short texturenum;

	unsigned int firstpoint;
	unsigned short numpoints;

} dface_t;

typedef struct {
	
	short planenum;

	short leafs[2];

	unsigned int firstpoint;
	unsigned short numpoints;

} dportal_t;

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
	short			contents;

	float		mins[3];	
	float		maxs[3];

	unsigned short	firstface;
	unsigned short	numfaces;

	unsigned short firstportal;
	unsigned short numportals;

} dleaf_t;

typedef struct {

	float vecs[2][4];
	int flags;
	int texturenum;

} texinfo_t;

extern int			  nummodels;
extern dmodel_t	      dmodels[MAX_MAP_MODELS];
					  
extern int			  lightdatasize;
extern byte		      dlightdata[MAX_MAP_LIGHTING];
					  
extern int			  entdatasize;
extern char		      dentdata[MAX_MAP_ENTSTRING];
					  
extern int			  numleafs;
extern dleaf_t		  dleafs[MAX_MAP_LEAFS];
					  
extern int			  numplanes;
extern dplane_t		  dplanes[MAX_MAP_PLANES];
					  
extern int			  numnodes;
extern dnode_t		  dnodes[MAX_MAP_NODES];
					  
extern int			  numtexinfo;
extern texinfo_t	  texinfo[MAX_MAP_TEXINFO];
					  
extern int			  numvert;
extern dvertex_t	  dvertexes[MAX_MAP_VERTS];

extern int			  numvertextable;
extern unsigned int   dvertextable[MAX_MAP_VERTEXTABLE];

extern int			  numfaces;
extern dface_t		  dfaces[MAX_MAP_FACES];

extern int			  numportals;
extern dportal_t	  dportals[MAX_MAP_PORTALS];

extern int			  numportaltable;
extern unsigned short dportaltable[MAX_MAP_PORTALTABLE]; 

extern int			  numclipnodes;
extern dclipnode_t	  dclipnodes[MAX_MAP_CLIPNODES];

extern FILE* bspfile;
extern char bspfilename[1024];

void BeginBSPFile(void);
void FinishBSPFile(void);

void PrintBSPFileSizes(void);