#pragma once

#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"

typedef struct {
	vec3_t	normal;
	vec_t	dist;
	int		type;
} plane_t;


#include "map.h"

#define	ON_EPSILON	0.01
#define	BOGUS_RANGE	36000
#define MAX_RANGE 9999999999999

#define MAX_WINDINGS_COUNT 128
#define MAX_POINTS_ON_WINDING 64


extern qbool verbose;
extern qbool noclip;
extern qbool nodraw;


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

	struct face_s* origin;

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
brushset_t* AllocBrushset(void);
void FreeBrushset(brushset_t* bs);

brush_t* LoadBrush(mbrush_t* mb, int hullnum);
brushset_t* Brush_LoadEntity(entity_t* ent, int hullnum);
int	FindPlane(plane_t* dplane, int* side);
//

typedef struct {

	vec3_t mins, maxs;
	vec3_t origin;

	brush_t* set;

} model_t;

typedef struct surface_s {

	struct surface_s* next;
	vec3_t mins, maxs;

	int planenum;
	face_t* faces;

	qbool onnode;

} surface_t;

surface_t* AllocSurface(void);
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

	int				contents;		

	// for leafes

	face_t** markfaces;	
	struct portal_s* portals;

	winding_t*		windings;
	int				visleafnum;		
	int				valid;			
	int				occupied;

} node_t;


node_t* AllocNode();
void FreeNode(node_t* node, qbool recursive);

surface_t* CSGFaces(brushset_t* bs);

extern brushset_t* brushset;

// solidbsp.c
void SubdivideFace(face_t* f, face_t** prevptr);

node_t* SolidBSP(surface_t* surfhead, qbool midsplit);



void qprintf(char* fmt, ...);