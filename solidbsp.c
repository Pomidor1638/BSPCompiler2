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

surface_t* ChooseMidPlaneFromList(surface_t* surfaces, vec3_t mins, vec3_t maxs) {

	int			j, l;
	surface_t* p, * bestsurface;
	vec_t		bestvalue, value, dist;
	plane_t* plane;

	//
	// pick the plane that splits the least
	//
	bestvalue = 6 * 8192 * 8192;
	bestsurface = NULL;

	for (p = surfaces; p; p = p->next)
	{
		if (p->onnode)
			continue;

		plane = &planes[p->planenum];

		// check for axis aligned surfaces
		plane;
		for (l = 0; l < 3; l++) 
			if (plane->normal[l] == 1)
				break;

		if (l == 3)
			continue;


		//
		// calculate the split metric along axis l, smaller values are better
		//
		value = 0;

		dist = plane->dist * plane->normal[l];
		for (j = 0; j < 3; j++)
		{
			if (j == l)
			{
				value += (maxs[l] - dist) * (maxs[l] - dist);
				value += (dist - mins[l]) * (dist - mins[l]);
			}
			else
				value += 2 * (maxs[j] - mins[j]) * (maxs[j] - mins[j]);
		}

		if (value > bestvalue)
			continue;

		//
		// currently the best!
		//
		bestvalue = value;
		bestsurface = p;
	}

	if (!bestsurface)
	{
		for (p = surfaces; p; p = p->next)
			if (!p->onnode)
				return p;		// first valid surface
		Error("ChooseMidPlaneFromList: no valid planes");
	}

	return bestsurface;
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

	if (usemidsplit) 
		return ChooseMidPlaneFromList(surfaces, mins, maxs);
	
	return ChoosePlaneFromList(surfaces, mins, maxs, qtrue);
}

void DividePlane(surface_t* in, plane_t* split, surface_t** front, surface_t** back) {

	face_t *frontlist, *backlist, *list, *f;
	face_t* next, *frontfrag, *backfrag;
	qbool onnode;

	plane_t* inplane;
	
	
	inplane = &planes[in->planenum];

	frontlist = backlist = NULL;

	if (VectorCompare(split->normal, inplane->normal)) {
		onnode = qfalse;
		if (split->dist == inplane->dist) {

			in->onnode = qtrue;

			for (f = in->faces; f; f = next) {
				next = f->next;
				if (f->planeside == 1) {
					f->next = backlist;
					backlist = f;
				}
				else {
					f->next = frontlist;
					frontlist = f;
				}
			}



			if (frontlist) {
				*front = CopySurface(in, qfalse);
				(*front)->faces = frontlist;
			}
			else
				*front = NULL;

			if (backlist) {
				*back = CopySurface(in, qfalse);
				(*back)->faces = backlist;
			}
			else
				*back = NULL;

			FreeSurface(in, qfalse);

			return;
		}

		if (inplane->dist > split->dist) {
			*front = CopySurface(in, qfalse);
			(*front)->faces = in->faces;
			*back = NULL;
		}
		else {
			*back = CopySurface(in, qfalse);
			(*back)->faces = in->faces;
			*front = NULL;
		}

		FreeSurface(in, qfalse);

		return;
	}


	for (f = in->faces; f; f = next) {
		next = f->next;
		SplitFace(f, split, &frontfrag, &backfrag);
		if (frontfrag) {
			frontfrag->next = frontlist;
			frontlist = frontfrag;
		}
		if (backfrag) {
			backfrag->next = backlist;
			backlist = backfrag;
		}
	}

	if (frontlist == NULL) {
		*front = NULL;
		*back = CopySurface(in, qfalse);
		(*back)->faces = backlist;

		FreeSurface(in, qfalse);

		return;
	}

	if (backlist == NULL) {
		*back = NULL;
		*front = CopySurface(in, qfalse);
		(*front)->faces = frontlist;

		FreeSurface(in, qfalse);

		return;
	}


	*front = CopySurface(in, qfalse);
	*back  = CopySurface(in, qfalse);

	(*front)->faces = frontlist;
	(*back)->faces  =  backlist;

	FreeSurface(in, qfalse);

	CalcSurfaceInfo(*front);
	CalcSurfaceInfo(*back);

}

void LinkConvexFaces(surface_t* planelist, node_t* leafnode) {

	qprintf("--------LinkConvexFaces------\n");
	face_t* f, * next;
	surface_t* surf, * pnext;
	int			i, count;
	//int fsc_surfCount;

	leafnode->contents = 0;
	leafnode->planenum = -1;

	count = 0;
	//fsc_surfCount = 0;
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
		//fsc_surfCount++;
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
	
	printf("LinkConvexFaces %i\n", c_empty + c_solid + c_water);

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
	face_t* f, * new, ** prevptr;
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

		new = CopyFace(f);
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
	int i;


	qprintf("-------PartitionSurfaces-------\n");
	qprintf("surface now: %p\n", surfaces);

	

	split = SelectPartition(surfaces);
	if (!split) {	// this is a leaf node
		node->planenum = PLANENUM_LEAF;
		LinkConvexFaces(surfaces, node);
		calls++;
		return;
	}

	qprintf("node maxs: %f, %f, %f\n", (float)node->maxs[0], (float)node->maxs[1], (float)node->maxs[2]);
	qprintf("node mins: %f, %f, %f\n", (float)node->mins[0], (float)node->mins[1], (float)node->mins[2]);

	splitnodes++;
	node->faces = LinkNodeFaces(split);
	node->children[0] = AllocNode();
	node->children[1] = AllocNode();
	node->planenum = split->planenum;

	splitplane = &planes[split->planenum];

	//DivideNodeBounds(node, splitplane);



	frontlist = NULL;
	backlist = NULL;

	for (p = surfaces; p; p = next)
	{
		next = p->next;
		DividePlane(p, splitplane, &frontfrag, &backfrag);

		if (frontfrag)
		{
			if (!frontfrag->faces)
				Error("surface with no faces");
			frontfrag->next = frontlist;
			frontlist = frontfrag;
		}
		if (backfrag)
		{
			if (!backfrag->faces)
				Error("surface with no faces");
			backfrag->next = backlist;
			backlist = backfrag;
		}
	}
	for (i = 0; i < 3; i++) {
		node->children[0]->maxs[i] = node->children[1]->maxs[i] = -MAX_RANGE;
		node->children[0]->mins[i] = node->children[1]->mins[i] =  MAX_RANGE;
	}

	for (p = frontlist; p; p = p->next) {
		for (i = 0; i < 3; i++) {
			if (p->maxs[i] > node->children[0]->maxs[i])
				node->children[0]->maxs[i] = p->maxs[i];
			if (p->mins[i] < node->children[0]->mins[i])
				node->children[0]->mins[i] = p->mins[i];

		}
	}

	for (p = backlist; p; p = p->next) {
		for (i = 0; i < 3; i++) {
			if (p->maxs[i] > node->children[1]->maxs[i])
				node->children[1]->maxs[i] = p->maxs[i];
			if (p->mins[i] < node->children[1]->mins[i])
				node->children[1]->mins[i] = p->mins[i];
		}
	}

	PartitionSurfaces(frontlist, node->children[0]);
	PartitionSurfaces(backlist, node->children[1]);
}



node_t* SolidBSP(surface_t* surfhead, qbool midsplit) {

	int		i;
	node_t* headnode;

	printf("----- SolidBSP -----\n");
	qprintf("-----midsplit= %d---\n", midsplit);

	headnode = AllocNode();
	usemidsplit = midsplit;

	for (i = 0; i < 3; i++) {
		headnode->mins[i] = brushset->mins[i] - 24;
		headnode->maxs[i] = brushset->maxs[i] + 24;
	}

	splitnodes = 0;
	leaffaces = 0;
	nodefaces = 0;
	c_solid = c_empty = c_water = 0;

	PartitionSurfaces(surfhead, headnode);

	i = TreeDepth(headnode);

	printf("--------Finished SolidBSP--------\n");
	printf("%5i split nodes\n", splitnodes);
	printf("%5i solid leafs\n", c_solid);
	printf("%5i empty leafs\n", c_empty);
	printf("%5i water leafs\n", c_water);
	printf("%5i leaffaces\n", leaffaces);
	printf("%5i nodefaces\n", nodefaces);
	printf("%5i treedepth\n", i);

	return headnode;
}


int __fastcall TreeDepth(node_t* node) {
	int n;
	if (!node)
		return 0;

	n = max(TreeDepth(node->children[0]), TreeDepth(node->children[1]));
	return 1 + n;
}