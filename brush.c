#include "qbsp.h"


vec3_t brush_mins, brush_maxs;

brushset_t* brushset;

int			numbrushplanes;
plane_t		planes[MAX_MAP_PLANES];

int			numbrushfaces;
mface_t		faces[MAX_FACES];

const vec3_t	hull_size[3][2] = {
	{ {  0,  0,  0}, {  0,  0,  0} },
	{ {-16,-16,-32}, { 16, 16, 24} },
	{ {-32,-32,-64}, { 32, 32, 24} }
};

#define	MAX_HULL_POINTS	32
#define	MAX_HULL_EDGES	64

int		num_hull_points;
vec3_t	hull_points[MAX_HULL_POINTS];
vec3_t	hull_corners[MAX_HULL_POINTS * 8];
int		num_hull_edges;
int		hull_edges[MAX_HULL_EDGES][2];

face_t* brush_faces;

#define	DISTEPSILON		0.005
#define	ANGLEEPSILON	0.00001


void ClearBounds(brushset_t* b) {
	for (int i = 0; i < 3; i++) {
		b->maxs[i] = -MAX_RANGE;
		b->mins[i] = MAX_RANGE;
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
	*side = 0;

	dp = planes;
	for (i = 0; i < numbrushplanes; i++, dp++) {
		dot = DotProduct(dp->normal, pl.normal);
		if (dot > 1.0 - ANGLEEPSILON && fabs(dp->dist - pl.dist) < DISTEPSILON) {
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
			&& fabs(pl->dist - plane->dist) < ON_EPSILON) {
			return;
		}
	}

	faces[i].plane = *plane;
	faces[i].texinfo = faces[0].texinfo;
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
			if (p1[j] > MAX_MAP_RANGE || p1[j] < -MAX_MAP_RANGE)
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

		CrossProduct(dir, facenormal, edgenormal);
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

int AddHullPoint(vec3_t p, int hullnum)
{
	int		i;
	vec_t* c;
	int		x, y, z;

	for (i = 0; i < num_hull_points; i++)
		if (VectorCompare(p, hull_points[i]))
			return i;

	VectorCopy(p, hull_points[num_hull_points]);

	c = hull_corners[i * 8];

	for (x = 0; x < 2; x++)
		for (y = 0; y < 2; y++)
			for (z = 0; z < 2; z++)
			{
				c[0] = p[0] + hull_size[hullnum][x][0];
				c[1] = p[1] + hull_size[hullnum][y][1];
				c[2] = p[2] + hull_size[hullnum][z][2];
				c += 3;
			}

	if (num_hull_points == MAX_HULL_POINTS)
		Error("MAX_HULL_POINTS");

	num_hull_points++;

	return i;
}

void TestAddPlane(plane_t* plane)
{
	int		i, c;
	vec_t	d;
	vec_t* corner;
	plane_t	flip;
	vec3_t	inv;
	int		counts[3];
	plane_t* pl;

	// see if the plane has allready been added
	for (i = 0; i < numbrushfaces; i++)
	{
		pl = &faces[i].plane;
		if (VectorCompare(plane->normal, pl->normal) && fabs(plane->dist - pl->dist) < ON_EPSILON)
			return;
		VectorSubtract(vec3_origin, plane->normal, inv);
		if (VectorCompare(inv, pl->normal) && fabs(plane->dist + pl->dist) < ON_EPSILON)
			return;
	}

	// check all the corner points
	counts[0] = counts[1] = counts[2] = 0;
	c = num_hull_points * 8;

	corner = hull_corners[0];
	for (i = 0; i < c; i++, corner += 3)
	{
		d = DotProduct(corner, plane->normal) - plane->dist;
		if (d < -ON_EPSILON)
		{
			if (counts[0])
				return;
			counts[1]++;
		}
		else if (d > ON_EPSILON)
		{
			if (counts[1])
				return;
			counts[0]++;
		}
		else
			counts[2]++;
	}

	// the plane is a seperator

	if (counts[0])
	{
		VectorSubtract(vec3_origin, plane->normal, flip.normal);
		flip.dist = -plane->dist;
		plane = &flip;
	}

	AddBrushPlane(plane);
}

void AddHullEdge(vec3_t p1, vec3_t p2, int hullnum)
{
	int		pt1, pt2;
	int		i;
	int		a, b, c, d, e;
	vec3_t	edgevec, planeorg, planevec;
	plane_t	plane;
	vec_t	l;

	pt1 = AddHullPoint(p1, hullnum);
	pt2 = AddHullPoint(p2, hullnum);

	for (i = 0; i < num_hull_edges; i++)
		if ((hull_edges[i][0] == pt1 && hull_edges[i][1] == pt2)
			|| (hull_edges[i][0] == pt2 && hull_edges[i][1] == pt1))
			return;	// allread added

	if (num_hull_edges == MAX_HULL_EDGES)
		Error("MAX_HULL_EDGES");

	hull_edges[i][0] = pt1;
	hull_edges[i][1] = pt2;
	num_hull_edges++;

	VectorSubtract(p1, p2, edgevec);
	VectorNormalize(edgevec);

	for (a = 0; a < 3; a++) {
		b = (a + 1) % 3;
		c = (a + 2) % 3;
		for (d = 0; d <= 1; d++)
			for (e = 0; e <= 1; e++) {
				VectorCopy(p1, planeorg);
				planeorg[b] += hull_size[hullnum][d][b];
				planeorg[c] += hull_size[hullnum][e][c];

				VectorCopy(vec3_origin, planevec);
				planevec[a] = 1;

				CrossProduct(planevec, edgevec, plane.normal);
				l = VectorNormalize(plane.normal);

				if (l < ANGLEEPSILON)
					continue;

				plane.dist = DotProduct(planeorg, plane.normal);
				TestAddPlane(&plane);
			}
	}
}


void ExpandBrush(int hullnum)
{
	int		i, x, s;
	vec3_t	corner;
	face_t* f;
	plane_t	plane, * p;

	num_hull_points = 0;
	num_hull_edges = 0;

	// create all the hull points
	for (f = brush_faces; f; f = f->next)
		for (i = 0; i < f->w->numpoints; i++)
			AddHullPoint(f->w->points[i], hullnum);

	// expand all of the planes
	for (i = 0; i < numbrushfaces; i++)
	{
		p = &faces[i].plane;
		VectorCopy(vec3_origin, corner);
		for (x = 0; x < 3; x++)
		{
			if (p->normal[x] > 0)
				corner[x] = hull_size[hullnum][1][x];
			else if (p->normal[x] < 0)
				corner[x] = hull_size[hullnum][0][x];
		}
		p->dist += DotProduct(corner, p->normal);
	}

	// add any axis planes not contained in the brush to bevel off corners
	for (x = 0; x < 3; x++)
		for (s = -1; s <= 1; s += 2)
		{
			// add the plane
			VectorCopy(vec3_origin, plane.normal);
			plane.normal[x] = s;
			if (s == -1)
				plane.dist = -brush_mins[x] + -hull_size[hullnum][0][x];
			else
				plane.dist = brush_maxs[x] + hull_size[hullnum][1][x];
			AddBrushPlane(&plane);
		}

	// add all of the edge bevels
	for (f = brush_faces; f; f = f->next)
		for (i = 0; i < f->w->numpoints; i++)
			AddHullEdge(f->w->points[i], f->w->points[(i + 1) % f->w->numpoints], hullnum);
}

#define	ZERO_EPSILON	0.0001
void CreateBrushFaces(void) {

	int		 i, j, k;
	vec_t		   r;
	face_t* f;
	winding_t* w;
	plane_t	   plane;
	mface_t* mf;

	if (brush_faces) {
		face_t* next;
		for (f = brush_faces; f; f = next) {
			next = f->next;
			FreeFace(f);
		}
	}

	brush_faces = NULL;

	for (int i = 0; i < 3; i++) {
		brush_mins[i] = MAX_RANGE;
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
		f->texturenum = FindTexinfo(&mf->texinfo);
		f->next = brush_faces;
		brush_faces = f;
		CheckFace(f);
	}
}


brush_t* LoadBrush(mbrush_t* mb, int hullnum) {

	qprintf("*--------------* LoadBrush *-------------*\n");

	face_t* f;
	brush_t* b;
	mface_t* mf;

	int i;

	for (i = 0; i < 3; i++) {
		brush_mins[i] =  MAX_RANGE;
		brush_maxs[i] = -MAX_RANGE;
	}

	numbrushfaces = 0;

	for (mf = mb->faces; mf; mf = mf->next) {
		faces[numbrushfaces] = *mf;
		if (hullnum)
			memset(&faces[numbrushfaces].texinfo, 0, sizeof(dtexinfo_t));
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

	i = 0;
	for (f = b->faces; f; f = f->next)
		i++;


	qprintf("%5i faces loaded\n", i);

	return b;
}

brushset_t* Brush_LoadEntity(entity_t* ent, int hullnum) {

	printf("*==========* Brush_LoadEntity *==========*\n");

	int bc;

	brushset_t* bs;
	brush_t* b;
	mbrush_t* mb;

	bs = AllocBrushset();
	bc = 0;

	for (mb = ent->brushes; mb; mb = mb->next) {
		b = LoadBrush(mb, hullnum);
		b->next = bs->brushes;
		bs->brushes = b;

		AddToBounds(bs, b->mins);
		AddToBounds(bs, b->maxs);
		bc++;
	}

	printf("%5i brushes\n", bc);

	return bs;
}
