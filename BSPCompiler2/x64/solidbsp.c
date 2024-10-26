#include "qbsp.h"

int		leaffaces;
int		nodefaces;
int		splitnodes;

int		c_solid, c_empty, c_water;

qbool		usemidsplit;


void SubdivideFace(face_t* f, face_t** prevptr) {

}

int FaceSide(face_t* in, plane_t* split) {
	int		frontcount, backcount;
	vec_t	dot;
	int		i;
	vec_t* p;


	frontcount = backcount = 0;

	
	for (i = 0, p = in->w->points[0]; i < in->w->numpoints; i++, p += 3) {
		dot = DotProduct(p, split->normal);
		dot -= split->dist;
		if (dot > ON_EPSILON) {
			if (backcount)
				return SIDE_CROSS;
			frontcount = 1;
		}
		else if (dot < -ON_EPSILON) {
			if (frontcount)
				return SIDE_CROSS;
			backcount = 1;
		}
	}

	if (!frontcount)
		return SIDE_BACK;
	if (!backcount)
		return SIDE_FRONT;

	return SIDE_ON;
}

surface_t* ChoosePlaneFromList(surface_t* surfaces, vec3_t mins, vec3_t maxs, qbool usefloors) {
	
	static const unsigned int CrossKoef = 2;
	static const unsigned int OnKoef = 0;
	static const unsigned int AbsKoef = 1;

	int			j, k, l;
	surface_t* p, * p2, * bestsurface;
	plane_t* plane;
	face_t* f;

	size_t back, cross, front, on;
	size_t bestvalue, value;

	bestvalue = MAX_RANGE;
	bestsurface = NULL;

	for (p = bestsurface; p; p = p->next) {
		
		if (p->onnode)
			continue;
		
		plane = &planes[p->planenum];
		front = cross = back = on = 0;

		for (p2 = bestsurface; p2; p2 = p2->next) {
			if (p2->onnode || p2 == p)
				continue;
			for (f = p2->faces; f; f = f->next) {
				switch (FaceSide(f, plane)) {

				case SIDE_ON:
					on++;
					break;
				case SIDE_BACK:
					back++;
					break;
				case SIDE_FRONT:
					front++;
					break;
				case SIDE_CROSS:
					cross++;
					break;
				default:
					break;

				}
			}

		}

		value = AbsKoef*abs(front - back) + OnKoef*on + CrossKoef*cross;

		if (value < bestvalue) {
			bestvalue = value;
			bestsurface = p;
		}
	}


	if (!bestsurface) {
		for (p = surfaces; p; p = p->next)
			if (!p->onnode)
				return p;		// first valid surface
		Error("ChooseMidPlaneFromList: no valid planes");
	}

	return bestsurface;
}

surface_t* SelectPartition(surface_t* surfaces) {

	int			i, j;
	vec3_t		mins, maxs;
	surface_t* p, * bestsurface;

	i = 0;
	bestsurface = NULL;
	for (p = surfaces; p; p = p->next) {
		if (!p->onnode) {
			i++;
			bestsurface = p;
		}
	}

	if (i == 0)
		return NULL; 

	if (i == 1)
		return bestsurface;	

	for (i = 0; i < 3; i++) {
		mins[i] =  MAX_RANGE;
		maxs[i] = -MAX_RANGE;
	}

	for (p = surfaces; p; p = p->next)
		for (j = 0; j < 3; j++) {
			if (p->mins[j] < mins[j])
				mins[j] = p->mins[j];
			if (p->maxs[j] > maxs[j])
				maxs[j] = p->maxs[j];
		}

	//if (usemidsplit) 
	//	return ChooseMidPlaneFromList(surfaces, mins, maxs);
	
	return ChoosePlaneFromList(surfaces, mins, maxs, qtrue);
}

void DividePlane(surface_t* in, plane_t* split, surface_t** front, surface_t** back)
{
	face_t* facet, * next;
	face_t* frontlist, * backlist;
	face_t* frontfrag, * backfrag;
	surface_t* news;
	plane_t* inplane;

	inplane = &planes[in->planenum];

	// parallel case is easy
	if (VectorCompare(inplane->normal, split->normal))
	{
		// check for exactly on node
		if (inplane->dist == split->dist)
		{	// divide the facets to the front and back sides
			news = AllocSurface();
			*news = *in;

			facet = in->faces;
			in->faces = NULL;
			news->faces = NULL;
			in->onnode = news->onnode = qtrue;

			for (; facet; facet = next)
			{
				next = facet->next;
				if (facet->planeside == 1)
				{
					facet->next = news->faces;
					news->faces = facet;
				}
				else
				{
					facet->next = in->faces;
					in->faces = facet;
				}
			}

			if (in->faces)
				*front = in;
			else
				*front = NULL;
			if (news->faces)
				*back = news;
			else
				*back = NULL;
			return;
		}

		if (inplane->dist > split->dist)
		{
			*front = in;
			*back = NULL;
		}
		else
		{
			*front = NULL;
			*back = in;
		}
		return;
	}

	// do a real split.  may still end up entirely on one side
	// OPTIMIZE: use bounding box for fast test
	frontlist = NULL;
	backlist = NULL;

	for (facet = in->faces; facet; facet = next)
	{
		next = facet->next;
		SplitFace(facet, split, &frontfrag, &backfrag);
		if (frontfrag)
		{
			frontfrag->next = frontlist;
			frontlist = frontfrag;
		}
		if (backfrag)
		{
			backfrag->next = backlist;
			backlist = backfrag;
		}
	}

	// if nothing actually got split, just move the in plane

	if (frontlist == NULL)
	{
		*front = NULL;
		*back = in;
		in->faces = backlist;
		return;
	}

	if (backlist == NULL)
	{
		*front = in;
		*back = NULL;
		in->faces = frontlist;
		return;
	}


	// stuff got split, so allocate one new plane and reuse in
	news = AllocSurface();
	*news = *in;
	news->faces = backlist;
	*back = news;

	in->faces = frontlist;
	*front = in;

	// recalc bboxes and flags
	CalcSurfaceInfo(news);
	CalcSurfaceInfo(in);
}

void LinkConvexFaces(surface_t* planelist, node_t* leafnode) {

	face_t* f, * next;
	surface_t* surf, * pnext;
	int			i, count;
	int fsc_surfCount;

	leafnode->contents = 0;
	leafnode->planenum = -1;

	count = 0;
	fsc_surfCount = 0;
	for (surf = planelist; surf; surf = surf->next)
	{

		for (f = surf->faces; f; f = f->next)
		{
			count++;
			if (!leafnode->contents)
				leafnode->contents = f->contents[0];
			else if (leafnode->contents != f->contents[0])
				Error("Mixed face contents in leafnode");
		}
		fsc_surfCount++;
	}

	if (!leafnode->contents)
		leafnode->contents = CONTENTS_SOLID;

	//Statistics.
	switch (leafnode->contents) {
	case CONTENTS_EMPTY:
		c_empty++;
		break;
	case CONTENTS_SOLID:
		c_solid++;
		break;
	case CONTENTS_WATER:
	case CONTENTS_SLIME:
	case CONTENTS_LAVA:
	case CONTENTS_SKY:
		c_water++;
		break;
	default:
		Error("LinkConvexFaces: bad contents number");
	}
	


	leaffaces += count;
	leafnode->markfaces = malloc(sizeof(face_t*) * (count + 1));
	i = 0;
	for (surf = planelist; surf; surf = pnext)
	{
		pnext = surf->next;
		for (f = surf->faces; f; f = next)
		{
			next = f->next;
			leafnode->markfaces[i] = f->original;
			i++;
			FreeFace(f);
		}
		FreeSurface(surf, qfalse);
	}
	leafnode->markfaces[i] = NULL;	// sentinal
}

face_t* LinkNodeFaces(surface_t* surface) {
	face_t* f, * new, **prevptr;
	face_t* list;

	list = NULL;


	prevptr = &surface->faces;
	while (1) {
		f = *prevptr;
		if (!f)
			break;
		SubdivideFace(f, prevptr);
		f = *prevptr;
		prevptr = &f->next;
	}


	for (f = surface->faces; f; f = f->next) {
		nodefaces++;
		new = AllocFace(f->w->numpoints);
		*new = *f;
		f->original = new;
		new->next = list;
		list = new;
	}

	return list;
}


void PartitionSurfaces(surface_t* surfaces, node_t* node) {
	
	surface_t* split, * p, * next;
	surface_t* frontlist, * backlist;
	surface_t* frontfrag, * backfrag;
	plane_t* splitplane;

	split = SelectPartition(surfaces);
	if (!split) {
		node->planenum = PLANENUM_LEAF;
		return;
	}




}



node_t* SolidBSP(surface_t* surfhead, qbool midsplit) {

	int		i;
	node_t* headnode;

	qprintf("----- SolidBSP -----\n");
	qprintf("-----midsplit= %d---\n", midsplit);

	headnode = AllocNode();
	usemidsplit = midsplit;

	for (i = 0; i < 3; i++) {
		headnode->mins[i] = brushset->mins[i];
		headnode->maxs[i] = brushset->maxs[i];
	}

	splitnodes = 0;
	leaffaces = 0;
	nodefaces = 0;
	c_solid = c_empty = c_water = 0;

	PartitionSurfaces(surfhead, headnode);

	printf("%5i split nodes\n", splitnodes);
	printf("%5i solid leafs\n", c_solid);
	printf("%5i empty leafs\n", c_empty);
	printf("%5i water leafs\n", c_water);
	printf("%5i leaffaces\n", leaffaces);
	printf("%5i nodefaces\n", nodefaces);

	return headnode;
}