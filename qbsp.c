#include "qbsp.h"

int NumWindings = 0;

winding_t* BaseWindingForPlane(plane_t* p) {
	int		i, x;
	vec_t	max, v;
	vec3_t	org, vright, vup;
	winding_t* w;

	// find the major axis

	max = -BOGUS_RANGE;
	x = -1;
	for (i = 0; i < 3; i++) {
		v = fabs(p->normal[i]);
		if (v > max) {
			x = i;
			max = v;
		}
	}
	if (x == -1)
		Error("BaseWindingForPlane: no axis found\n");

	VectorCopy(vec3_origin, vup);
	switch (x) {
	case 0:
	case 1:
		vup[2] = 1;
		break;
	case 2:
		vup[0] = 1;
		break;
	}

	v = DotProduct(vup, p->normal);
	VectorMA(vup, -v, p->normal, vup);
	VectorNormalize(vup);

	VectorScale(p->normal, p->dist, org);

	CrossProduct(vup, p->normal, vright);

	VectorScale(vup, 8192, vup);
	VectorScale(vright, 8192, vright);

	// project a really big	axis aligned box onto the plane
	w = AllocWinding(4);

	VectorSubtract(org, vright, w->points[0]);
	VectorAdd(w->points[0], vup, w->points[0]);

	VectorAdd(org, vright, w->points[1]);
	VectorAdd(w->points[1], vup, w->points[1]);

	VectorAdd(org, vright, w->points[2]);
	VectorSubtract(w->points[2], vup, w->points[2]);

	VectorSubtract(org, vright, w->points[3]);
	VectorSubtract(w->points[3], vup, w->points[3]);

	w->numpoints = 4;

	return w;
}

winding_t* AllocWinding(int num) {
	winding_t* w;
	
	//if (NumWindings >= MAX_WINDINGS_COUNT || num > MAX_POINTS_ON_WINDING)
	//	return NULL;

	NumWindings++;

	if (num >= MAX_POINTS_ON_WINDING)
		Error("AllocWinding: MAX_POINTS_ON_WINDING\n");

	w = malloc(sizeof(winding_t));
	w->points = malloc(num * sizeof(vec3_t));

	w->numpoints = 0;

	return w;
}

void FreeWinding(winding_t* w) {
	
	if (!w) return;
	NumWindings--;
	free(w->points);
	free(w);
}

winding_t* CopyWinding(winding_t* w) {
	int			size;
	winding_t* c;

	NumWindings++;

	size = w->numpoints * sizeof(vec3_t);

	c = malloc(sizeof(winding_t));
	c->points = malloc(size);

	c->numpoints = w->numpoints;

	memcpy(c->points, w->points, size);


	return c;
}

winding_t* ClipWinding(winding_t* in, plane_t* split, qbool keepon) {
	vec_t	dists[MAX_POINTS_ON_WINDING];
	int		sides[MAX_POINTS_ON_WINDING];
	int		counts[3];
	vec_t	dot;
	int		i, j;
	vec_t* p1, * p2;
	vec3_t	mid;
	winding_t* neww;
	int		maxpts;

	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	for (i = 0; i < in->numpoints; i++)
	{
		dot = DotProduct(in->points[i], split->normal);
		dot -= split->dist;
		dists[i] = dot;
		if (dot > ON_EPSILON)
			sides[i] = SIDE_FRONT;
		else if (dot < -ON_EPSILON)
			sides[i] = SIDE_BACK;
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if (keepon && !counts[0] && !counts[1])
		return in;

	if (!counts[0])
	{
		FreeWinding(in);
		return NULL;
	}
	if (!counts[1])
		return in;

	maxpts = in->numpoints + 4;	

	neww = AllocWinding(maxpts);

	for (i = 0; i < in->numpoints; i++)
	{
		p1 = in->points[i];

		if (sides[i] == SIDE_ON)
		{
			VectorCopy(p1, neww->points[neww->numpoints]);
			neww->numpoints++;
			continue;
		}

		if (sides[i] == SIDE_FRONT)
		{
			VectorCopy(p1, neww->points[neww->numpoints]);
			neww->numpoints++;
		}

		if (sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i])
			continue;

		// generate a split point
		p2 = in->points[(i + 1) % in->numpoints];

		dot = dists[i] / (dists[i] - dists[i + 1]);
		for (j = 0; j < 3; j++)
		{	// avoid round off error when possible
			if (split->normal[j] == 1)
				mid[j] = split->dist;
			else if (split->normal[j] == -1)
				mid[j] = -split->dist;
			else
				mid[j] = p1[j] + dot * (p2[j] - p1[j]);
		}

		VectorCopy(mid, neww->points[neww->numpoints]);
		neww->numpoints++;
	}

	//if (neww->numpoints > maxpts)
		//Error("ClipWinding: points exceeded estimate");

	// free the original winding

	FreeWinding(in);
	in = CopyWinding(neww);
	FreeWinding(neww);
	neww = in;

	return neww;
}

void	DivideWinding(winding_t* in, plane_t* split, winding_t** front, winding_t** back) {
	vec_t	dists[MAX_POINTS_ON_WINDING];
	int		sides[MAX_POINTS_ON_WINDING];
	int		counts[3];
	vec_t	dot;
	int		i, j;
	vec_t* p1, * p2;
	vec3_t	mid;
	winding_t* f, *b;
	int		maxpts;
	int f_count, b_count;


	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	for (i = 0; i < in->numpoints; i++)
	{
		dot = DotProduct(in->points[i], split->normal);
		dot -= split->dist;
		dists[i] = dot;
		if (dot > ON_EPSILON)
			sides[i] = SIDE_FRONT;
		else if (dot < -ON_EPSILON)
			sides[i] = SIDE_BACK;
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	*front = *back = NULL;

	if (!counts[0])
	{
		*back = in;
		return;
	}
	if (!counts[1])
	{
		*front = in;
		return;
	}

	maxpts = in->numpoints + 4;	// can't use counts[0]+2 because
	// of fp grouping errors

	f = AllocWinding(maxpts);
	b = AllocWinding(maxpts);

	for (i = 0; i < in->numpoints; i++)
	{
		p1 = in->points[i];

		if (sides[i] == SIDE_ON)
		{
			VectorCopy(p1, f->points[f->numpoints]);
			f->numpoints++;
			VectorCopy(p1, b->points[b->numpoints]);
			b->numpoints++;
			continue;
		}

		if (sides[i] == SIDE_FRONT)
		{
			VectorCopy(p1, f->points[f->numpoints]);
			f->numpoints++;
		}
		if (sides[i] == SIDE_BACK)
		{
			VectorCopy(p1, b->points[b->numpoints]);
			b->numpoints++;
		}

		if (sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i])
			continue;

		p2 = in->points[(i + 1) % in->numpoints];

		dot = dists[i] / (dists[i] - dists[i + 1]);

		for (j = 0; j < 3; j++) {
			if (split->normal[j] == 1)
				mid[j] = split->dist;
			else if (split->normal[j] == -1)
				mid[j] = -split->dist;
			else
				mid[j] = p1[j] + dot * (p2[j] - p1[j]);
		}

		VectorCopy(mid, f->points[f->numpoints]);
		f->numpoints++;
		VectorCopy(mid, b->points[b->numpoints]);
		b->numpoints++;
	}

	if (f->numpoints > maxpts || b->numpoints > maxpts)
		Error("ClipWinding: points exceeded estimate\n");

	*front = CopyWinding(f);
	*back = CopyWinding(b);

	FreeWinding(f);
	FreeWinding(b);

}


face_t* AllocFace(int num) {
	
	face_t* f;

	f = malloc(sizeof(face_t));
	memset(f, 0, sizeof(face_t));
	f->planenum = -1;

	f->w = AllocWinding(num);
	f->w->numpoints = num;

	return f;
}


void FreeFace(face_t* f) {
	facecount--;
	FreeWinding(f->w);
	free(f);
}

face_t* CopyFace(face_t* f) {
	face_t* c;
	
	facecount++;

	c = malloc(sizeof(face_t));
	memcpy(c, f, sizeof(face_t));

	f->w = CopyWinding(f->w);
}

brush_t* AllocBrush(void) {
	brush_t* b;

	b = malloc(sizeof(brush_t));
	memset(b, 0, sizeof(brush_t));

	return b;
}


