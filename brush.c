#include "qbsp.h"


vec3_t brush_mins, brush_maxs;

int			numbrushplanes;
plane_t		planes[MAX_MAP_PLANES];

int			numbrushfaces;
mface_t		faces[MAX_FACES];

vec3_t	hull_size[3][2] = {
	{ {  0,  0,  0}, {  0,  0,  0} },
	{ {-16,-16,-32}, { 16, 16, 24} },
	{ {-32,-32,-64}, { 32, 32, 24} }
};

face_t* brush_faces;

#define	DISTEPSILON		0.005
#define	ANGLEEPSILON	0.00001


void ClearBounds(brushset_t* b) {
	for (int i = 0; i < 3; i++) {
		b->maxs[i] = -MAX_RANGE;
		b->mins[i] =  MAX_RANGE;
	}
}

void AddToBounds(brushset_t* b, vec3_t p) {
	for (int i = 0; i < 3; i++) {
		if (p[i] < b->mins[i])
			b->mins[i] = p[i];
		if (p[i] > b->maxs[i])
			b->maxs[i] = p[i];
	}
}

int	FindPlane(plane_t* dplane, int* side) {
	int			i;
	plane_t* dp, pl;
	double		dot;


	dot = VectorLength(dplane->normal);
	if (dot < 1.0 - ANGLEEPSILON || dot > 1.0 + ANGLEEPSILON)
		Error("FindPlane: normalization error");

	pl = *dplane;

	dp = planes;
	for (i = 0; i < numbrushplanes; i++, dp++) {
		dot = DotProduct(dp->normal, pl.normal);
		if (dot > 1.0 - ANGLEEPSILON && fabs(dp->dist - pl.dist) < DISTEPSILON) {
			*side = 0;
			return i;
		}
		
		if (dot < -1.0 + ANGLEEPSILON && fabs(dp->dist + pl.dist) < DISTEPSILON) {
			*side = 1;
			return i;
		}
	}

	if (numbrushplanes == MAX_MAP_PLANES)
		Error("numbrushplanes == MAX_MAP_PLANES");

	planes[numbrushplanes] = pl;
	numbrushplanes++;

	return numbrushplanes - 1;
}

void AddBrushPlane(plane_t* plane) {
	int		i;
	plane_t* pl;
	float	l;

	if (numbrushfaces == MAX_FACES)
		Error("AddBrushPlane: numbrushfaces == MAX_FACES");
	l = VectorLength(plane->normal);
	if (l < 1 - LENGTH_EPSILON || l > 1 + LENGTH_EPSILON)
		Error("AddBrushPlane: bad normal");

	for (i = 0; i < numbrushfaces; i++) {

		pl = &faces[i].plane;
		if (VectorCompare(pl->normal, plane->normal) 
			&& fabs(pl->dist - plane->dist) < ON_EPSILON){
			return;
		}
	}

	faces[i].plane = *plane;
	faces[i].texinfo = 0; // faces[0].texinfo;
	numbrushfaces++;
}

void CheckFace(face_t* f)
{
	int		i, j;
	vec_t* p1, * p2;
	vec_t	d, edgedist;
	vec3_t	dir, edgenormal, facenormal;

	if (f->w->numpoints < 3)
		Error("CheckFace: %i points", f->w->numpoints);

	VectorCopy(planes[f->planenum].normal, facenormal);
	if (f->planeside)
	{
		VectorSubtract(vec3_origin, facenormal, facenormal);
	}

	for (i = 0; i < f->w->numpoints; i++)
	{
		p1 = f->w->points[i];

		for (j = 0; j < 3; j++)
			if (p1[j] > BOGUS_RANGE || p1[j] < -BOGUS_RANGE)
				Error("CheckFace: BUGUS_RANGE: %f", p1[j]);

		j = i + 1 == f->w->numpoints ? 0 : i + 1;

		// check the point is on the face plane
		d = DotProduct(p1, planes[f->planenum].normal) - planes[f->planenum].dist;
		if (d < -ON_EPSILON || d > ON_EPSILON)
			Error("CheckFace: point off plane");

		// check the edge isn't degenerate
		p2 = f->w->points[j];
		VectorSubtract(p2, p1, dir);

		if (VectorLength(dir) < ON_EPSILON)
			Error("CheckFace: degenerate edge");

		CrossProduct(facenormal, dir, edgenormal);
		VectorNormalize(edgenormal);
		edgedist = DotProduct(p1, edgenormal);
		edgedist += ON_EPSILON;

		// all other points must be on front side
		for (j = 0; j < f->w->numpoints; j++)
		{
			if (j == i)
				continue;
			d = DotProduct(f->w->points[j], edgenormal);
			if (d > edgedist)
				Error("CheckFace: non-convex");
		}
	}
}

void ExpandBrush(int hullnum) {
	int i, j;
	vec_t dot;
	vec3_t corner;
	plane_t *p, plane;
	mface_t* mf;

	for (i = 0; i < numbrushfaces; i++) {
		mf = &faces[i];
		p = &mf->plane;

		VectorCopy(vec3_origin, corner);

		for (j = 0; j < 3; j++) {
			if (p->normal[j] > 0)
				corner[j] = hull_size[hullnum][1][j];
			else if (p->normal[j] < 0)
				corner[j] = hull_size[hullnum][0][j];
		}
		p->dist += DotProduct(p->normal, corner);

		mf->texinfo = 0;


	}

	for (i = 0; i < 3; i++) {
		for (j = -1; j <= 1; j += 2) {

			VectorCopy(vec3_origin, plane.normal);
			plane.normal[i] = j;
			if (j == -1)
				plane.dist = -brush_mins[i] + -hull_size[hullnum][0][i];
			else
				plane.dist = brush_maxs[i] + hull_size[hullnum][1][i];

			//printf("plane: %f, %f, %f, %f\n ", plane.normal[0], plane.normal[1], plane.normal[2], plane.dist);
			AddBrushPlane(&plane);
		}
	}
}

#define	ZERO_EPSILON	0.0001
void CreateBrushFaces(void) {

	int		 i, j, k;
	vec_t		   r;
	face_t*		   f;
	winding_t*     w;
	plane_t	   plane;
	mface_t*      mf;

	if (brush_faces) {
		face_t* next;
		for (f = brush_faces; f; f = next) {
			next = f->next;
			FreeFace(f);
		}
	}

	brush_faces = NULL;

	for (int i = 0; i < 3; i++) {
		brush_mins[i] =  MAX_RANGE;
		brush_maxs[i] = -MAX_RANGE;
	}

	for (i = 0; i < numbrushfaces; i++) {
		mf = &faces[i];

		w = BaseWindingForPlane(&mf->plane);

		for (j = 0; j < numbrushfaces && w; j++) {

			if (i == j)
				continue;

			VectorSubtract(vec3_origin, faces[j].plane.normal, plane.normal);
			plane.dist = -faces[j].plane.dist;

			w = ClipWinding(w, &plane, qfalse);
		}

		if (!w) 
			continue;



		f = AllocFace(w->numpoints);

		for (j = 0; j < f->w->numpoints; j++) {

			for (k = 0; k < 3; k++) {

				r = Q_round(w->points[j][k]);
				if (fabs(w->points[j][k] - r) < ZERO_EPSILON)
					f->w->points[j][k] = r;
				else
					f->w->points[j][k] = w->points[j][k];

				if (f->w->points[j][k] < brush_mins[k])
					brush_mins[k] = f->w->points[j][k];
				if (f->w->points[j][k] > brush_maxs[k])
					brush_maxs[k] = f->w->points[j][k];

			}
		}

		FreeWinding(w);

		f->planenum = FindPlane(&mf->plane, &f->planeside);
		f->texturenum = mf->texinfo;
		f->next = brush_faces;
		brush_faces = f;
		CheckFace(f);
	}
}


brush_t* LoadBrush(mbrush_t* mb, int hullnum) {

	qprintf("----------LoadBrush---------\n");

	brush_t* b;
	mface_t* mf;
	

	for (int i = 0; i < 3; i++) {
		brush_mins[i] =  MAX_RANGE;
		brush_maxs[i] = -MAX_RANGE;
	}

	numbrushfaces = 0;


	for (mf = mb->faces; mf; mf = mf->next) {
		faces[numbrushfaces] = *mf;
		if (hullnum)
			faces[numbrushfaces].texinfo = 0;
		numbrushfaces++;
	}

	brush_faces = NULL;

	CreateBrushFaces();

	if (hullnum) {
		ExpandBrush(hullnum);
		CreateBrushFaces();
	}


	if (brush_faces == NULL) {
		printf("Error: couldn`t create brush_faces\n");
		return NULL;
	}

	b = AllocBrush();
	VectorCopy(brush_mins, b->mins);
	VectorCopy(brush_maxs, b->maxs);

	b->contents = mb->contents;
	b->faces = brush_faces;

	return b;
}

brushset_t* Brush_LoadEntity(entity_t* ent, int hullnum) {

	qprintf("------------Brush_LoadEntity---------\n");

	brushset_t* bs;
	brush_t* b;
	mbrush_t* mb;

	bs = AllocBrushset();

	for (mb = ent->brushes; mb; mb = mb->next) {
		b = LoadBrush(mb, hullnum);
		b->next = bs->brushes;
		bs->brushes = b;

		AddToBounds(bs, b->mins);
		AddToBounds(bs, b->maxs);
	}
	
	planecount = numbrushfaces;

	return bs;
}
