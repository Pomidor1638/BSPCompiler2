#pragma once
#include <string>
#include <cstdint> // Äëÿ uint16_t, uint32_t

typedef unsigned char byte;

typedef float dvec_t;
typedef dvec_t dvec3_t[3];

#define PLANENUM_LEAF        -1

#define MAX_MAP_HULLS        4

#define MAX_MAP_MODELS       256
#define MAX_MAP_BRUSHES      4096
#define MAX_MAP_ENTITIES     1024
#define MAX_MAP_ENTSTRING    65536

#define MAX_MAP_PLANES       16384
#define MAX_MAP_NODES        16384
#define MAX_MAP_CLIPNODES    16384
#define MAX_MAP_LEAFS        16384    
#define MAX_MAP_VERTEXES     32768
#define MAX_MAP_VERTEXINDEXES 65536*2
#define MAX_MAP_FACES        32768    
#define MAX_MAP_PORTALS      32768
#define MAX_MAP_PORTALINDEXES 32768
#define MAX_MAP_TEXINFO      4096
#define MAX_MAP_TEXTURES     256
#define MAX_TEXTURE_NAME     32
#define MIP_MAP_LEVELS       4
#define MAX_MAP_LIGHTING     0x100000

#define MAX_POINTS_ON_WINDING 64
#define MAX_FACES            128

#define MAX_KEY        32
#define MAX_VALUE      1024


#define BSPVERSION            133


enum LUMP_THINGS {

	LUMP_PLANES = 0,
	LUMP_VERTEXES,
	LUMP_VINDEXES,
	LUMP_PORTALS,
	LUMP_PINDEXES,
	LUMP_FACES,
	LUMP_NODES,
	LUMP_LEAFS,
	LUMP_CLIPNODES,
	LUMP_TEXINFO,
	LUMP_TEXTURES,
	LUMP_LIGHTING,
	LUMP_VISIBILITY,
	LUMP_ENTITIES,
	LUMP_MODELS,

	HEADER_LUMPS
};

#pragma pack(push, 1)

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

typedef enum CONTENTS_TYPE {
	CONTENTS_EMPTY = -1,
	CONTENTS_SOLID = -2,
	CONTENTS_WATER = -3,
	CONTENTS_SLIME = -4,
	CONTENTS_LAVA = -5,
	CONTENTS_SKY = -6,
} CONTENTS_TYPE;

typedef struct {
	float normal[3];
	float  dist;
} dplane_t;

typedef struct dvertex_s {
	float v[3];
} dvertex_t;

typedef struct {

	short planenum;
	byte side;
	short texinfonum;

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
	short           contents;
	float           mins[3];
	float           maxs[3];
	unsigned int    firstface;
	unsigned short  numfaces;
	unsigned int    firstportal;
	unsigned short  numportals;

} dleaf_t;


typedef struct {
	float vecs[2][4];
	int miptexnum;
	int flags;
} dtexinfo_t;

typedef struct
{
	int			nummiptex;
	int			dataofs[4];		// [nummiptex]
} dmiptexlump_t;

#define	MIPLEVELS	4
typedef struct miptex_s
{
	char		name[MAX_TEXTURE_NAME];
	unsigned	width, height;
	unsigned	offsets[MIPLEVELS];		// four mip maps stored
	byte		colortype;
} miptex_t;

#pragma pack(pop) 

struct BSPMap {
    std::string     bspfilename;

    int             nummodels = 0;
    dmodel_t* dmodels = nullptr;

    int             lightdatasize = 0;
    byte* dlightdata = nullptr;

	int             texdatasize = 0;
	byte* dtexdata = nullptr;

    int             entdatasize = 0;
    byte* dentdata = nullptr;

    int             texturedatasize = 0;
    byte* dtexturedata = nullptr;

    int             numleafs = 0;
    dleaf_t* dleafs = nullptr;

    int             numplanes = 0;
    dplane_t* dplanes = nullptr;

    int             numnodes = 0;
    dnode_t* dnodes = nullptr;

    int             numtexinfo = 0;
    dtexinfo_t* dtexinfo = nullptr;

    int             numvertexes = 0;
    dvertex_t* dvertexes = nullptr;

    int             numvindexes = 0;
    unsigned int* dvindexes = nullptr;

    int             numfaces = 0;
    dface_t* dfaces = nullptr;

    int             numportals = 0;
    dportal_t* dportals = nullptr;

    int             numpindexes = 0;
    unsigned short* dpindexes = nullptr;

    int             numclipnodes = 0;
    dclipnode_t* dclipnodes = nullptr;

    bool loadMap(std::string path);
    void printMap() const;

    ~BSPMap();
};