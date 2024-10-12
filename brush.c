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


void ClearBounds(void) {
	for (int i = 0; i < 3; i++) {
		brush_maxs[i] = -9999999;
		brush_mins[i] =  9999999;
	}
}

void AddToBounds(vec3_t p) {
	for (int i = 0; i < 3; i++) {
		if (p[i] < brush_mins[i])
			brush_mins[i] = p[i];
		if (p[i] > brush_maxs[i])
			brush_maxs[i] = p[i];
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
	//printf("1\n");
;	//printf("Plane Added: %f, %f, %f, %f\n", plane->normal[0], plane->normal[1], plane->normal[2], plane->dist);

	faces[i].plane = *plane;
	faces[i].texinfo = 0; // faces[0].texinfo;
	numbrushfaces++;
}

void CheckFace(face_t* f) {
	
	int num;
	vec3_t* points;
	vec3_t dir;
	vec_t dot;


	num = f->w->numpoints;

	if (num < 3)
		Error("CheckFace: %i points", f->w->numpoints);

	points = f->w->points;

	for (int i = 0; i < num; i++) {
		VectorSubtract(points[i], points[(i + 1) % num], dir);
		if (VectorLength(dir) < ON_EPSILON)
			Error("CheckFace: degenerate edge");
		dot = DotProduct(planes[f->planenum].normal, points[i]) - planes[f->planeside].dist;
		if (dot < ON_EPSILON)
			Error("CheckFace: bad points");
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

	brush_faces = NULL;

	ClearBounds();

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
	}
}


brush_t* LoadBrush(mbrush_t* mb, int hullnum) {
	brush_t* b;
	mface_t* mf;

	ClearBounds();

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

	b->faces = brush_faces;
	//b->numfaces 
	return b;
}