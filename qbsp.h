#pragma once

#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"

typedef struct {
	vec3_t	normal;
	vec_t	dist; 
} plane_t;


#include "map.h"

#define	ON_EPSILON	 0.001
#define	MAX_MAP_RANGE 16384
#define MAX_RANGE   999999

#define MAX_WINDINGS_COUNT    128
#define MAX_POINTS_ON_WINDING  64

#define SIDESPACE 24

// qbsp.c 

extern qbool verbose;
extern qbool noclip;
extern qbool nodraw;

void qprintf(char* fmt, ...);

extern int NumWindings;

typedef struct winding_s {

	int numpoints;
	vec3_t* points;

} winding_t;

winding_t* AllocWinding(int num);
winding_t* BaseWindingForPlane(plane_t* p);
void FreeWinding(winding_t* w);
winding_t* CopyWinding(winding_t* w);
winding_t* ClipWinding(winding_t* in, plane_t* split, qbool keepon);
void	DivideWinding(winding_t* in, plane_t* split, winding_t** front, winding_t** back);


extern	int			numbrushplanes;
extern	plane_t		planes[MAX_MAP_PLANES];


typedef struct face_s {

	struct face_s* next;

	int	planenum;
	int	planeside;
	int	texturenum;
	int	contents[2];	
	
	struct face_s* original;

	winding_t* w;

	qbool marked;

} face_t;


face_t* AllocFace(int num);
void FreeFace(face_t* f);
face_t* CopyFace(face_t* f);
face_t* ExtendFace(face_t* f);

typedef struct brush_s  {
	struct brush_s* next;
	
	vec3_t mins, maxs;
	int numfaces;
	int contents;

	face_t* faces;

} brush_t;

brush_t* AllocBrush(void);
void FreeBrush(brush_t* b);

typedef struct {
	
	vec3_t mins, maxs;
	brush_t* brushes;

} brushset_t;

// brush.c

extern brushset_t* brushset;

brushset_t* AllocBrushset(void);
void FreeBrushset(brushset_t* bs);

brush_t* LoadBrush(mbrush_t* mb, int hullnum);
brushset_t* Brush_LoadEntity(entity_t* ent, int hullnum);
int	FindPlane(plane_t* dplane, int* side);

typedef struct surface_s {

	struct surface_s* next;
	vec3_t mins, maxs;

	int planenum;
	face_t* faces;

	qbool onnode;

} surface_t;


surface_t* AllocSurface(void);
surface_t* CopySurface(surface_t* surface, qbool copyfaces);
void FreeSurface(surface_t* surf, qbool freeface);

// csg.c

extern	face_t* validfaces[MAX_MAP_PLANES];
surface_t* BuildSurfaces(void);
void SplitFace(face_t* in, plane_t* split, face_t** front, face_t** back);
void CalcSurfaceInfo(surface_t* surf);
//


typedef struct node_s {

	vec3_t			mins, maxs;	

	int				planenum;	
	int				outputplanenum;

	struct node_s* children[2];	
	
	face_t* faces;			

	int				contents;	
	
	face_t** markfaces;	
	
	struct portal_s* portals;
	int		   outputleafnum;
	int				valid;		
	int				occupied;	

} node_t;


node_t* AllocNode();
void FreeNode_r(node_t* node);

surface_t* CSGFaces(brushset_t* bs);

// solidbsp.c
void SubdivideFace(face_t* f, face_t** prevptr);

node_t* SolidBSP(surface_t* surfhead, qbool midsplit, qbool final);
int TreeDepth(node_t* node);

// portal.c
typedef struct portal_s {
	int			planenum;
	node_t* nodes[2];		
	struct portal_s* next[2];	
	winding_t* winding;
} portal_t;

extern	node_t	outside_node;		

void PortalizeWorld(node_t* headnode); 
void WritePortals(node_t* headnode);
void FreeAllPortals(node_t* node);

portal_t* AllocPortal(void);
void FreePortal(portal_t* p);


// writebsp.c

void WriteNodePlanes(node_t* headnode);
int FindFinalVertex(vec3_t v);
void WriteClipNodes(node_t* headnode);
void BumpModel(int hullnum);
void WriteDrawNodes(node_t* headnode);
int FindFinalPlane(dplane_t* p);