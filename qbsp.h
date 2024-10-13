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

#define MAX_WINDINGS_COUNT 128
#define MAX_POINTS_ON_WINDING 64

extern int NumWindings;

typedef struct winding_s {

	int numpoints;
	vec3_t* points;

} winding_t;

extern winding_t* AllocWinding(int num);
extern winding_t* BaseWindingForPlane(plane_t* p);
extern void FreeWinding(winding_t* w);
extern winding_t* CopyWinding(winding_t* w);
extern winding_t* ClipWinding(winding_t* in, plane_t* split, qbool keepon);
extern void	DivideWinding(winding_t* in, plane_t* split, winding_t** front, winding_t** back);

extern	int			numbrushplanes;
extern	plane_t		planes[MAX_MAP_PLANES];

typedef struct face_s {
	struct face_s* next;

	int	planenum;
	int	planeside;
	int	texturenum;
	int	contents[2];	

	winding_t* w;

} face_t;

face_t* AllocFace(int num);
void FreeFace(face_t* f);
face_t* CopyFace(face_t* f);


typedef struct brush_s  {
	struct brush_s* next;
	
	vec3_t mins, maxs;
	int numfaces;
	int contents;

	face_t* faces;

} brush_t;

brush_t* AllocBrush();

typedef struct {
	
	vec3_t mins, maxs;
	brush_t* brushes;

} brushset_t;

brush_t* LoadBrush(mbrush_t* mb, int hullnum);
brushset_t* Brush_LoadEntity(entity_t* ent, int hullnum);
int	FindPlane(plane_t* dplane, int* side);


typedef struct {

	vec3_t mins, maxs;
	vec3_t origin;

	brush_t* set;

} model_t;

typedef struct surface_s {

	struct surface_s* next;
	int planenum;
	face_t* faceset;

} surface_t;


typedef struct node_s {

	vec3_t mins, maxs;
	int planenum;

	surface_t* face;

	struct node_s* children[2];

} node_t;