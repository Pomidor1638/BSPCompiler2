#include "qbsp.h"

int		headclipnode;
int		firstface;

dplane_t PlaneTodPlane(plane_t p) {
	dplane_t dp;

	dp.dist = p.dist;

	dp.normal[0] = p.normal[0];
	dp.normal[1] = p.normal[1];
	dp.normal[2] = p.normal[2];
	
	return dp;
}

int FindFinalPlane(dplane_t* p) {

	int		i;
	dplane_t* dplane;

	for (i = 0, dplane = dplanes; i < numplanes; i++, dplane++) {
		if (p->dist != dplane->dist)
			continue;
		if (p->normal[0] != dplane->normal[0])
			continue;
		if (p->normal[1] != dplane->normal[1])
			continue;
		if (p->normal[2] != dplane->normal[2])
			continue;
		return i;
	}

	if (numplanes == MAX_MAP_PLANES)
		Error("numplanes == MAX_MAP_PLANES");
	dplane = &dplanes[numplanes];
	*dplane = *p;
	numplanes++;

	return numplanes - 1;
}


int		planemapping[MAX_MAP_PLANES];

void WriteNodePlanes_r(node_t* node)
{
	plane_t* plane;
	dplane_t* dplane;

	if (node->planenum == -1)
		return;
	if (planemapping[node->planenum] == -1)
	{	// a new plane
		planemapping[node->planenum] = numplanes;

		if (numplanes == MAX_MAP_PLANES)
			Error("numplanes == MAX_MAP_PLANES");
		plane = &planes[node->planenum];
		dplane = &dplanes[numplanes];
		dplane->normal[0] = plane->normal[0];
		dplane->normal[1] = plane->normal[1];
		dplane->normal[2] = plane->normal[2];
		dplane->dist = plane->dist;

		numplanes++;
	}

	node->outputplanenum = planemapping[node->planenum];

	WriteNodePlanes_r(node->children[0]);
	WriteNodePlanes_r(node->children[1]);
}


void WriteNodePlanes(node_t* headnode) {
	memset(planemapping, -1, sizeof(planemapping));
	WriteNodePlanes_r(headnode);
}

int WriteClipNodes_r(node_t* node)
{
	int			i, c;
	dclipnode_t* cn;
	int			num;

	// FIXME: free more stuff?	
	if (node->planenum == -1)
	{
		num = node->contents;
		free(node);
		return num;
	}

	// emit a clipnode
	c = numclipnodes;
	cn = &dclipnodes[numclipnodes];
	numclipnodes++;
	cn->planenum = node->outputplanenum;
	for (i = 0; i < 2; i++)
		cn->children[i] = WriteClipNodes_r(node->children[i]);

	free(node);
	return c;
}

void WriteClipNodes(node_t* nodes) {
	headclipnode = numclipnodes;
	WriteClipNodes_r(nodes);
}

void WriteLeaf(node_t* node) {

	int i;
	face_t* f;
	dleaf_t* leaf_p;
	dface_t* df;
	dplane_t dp;

	leaf_p = &dleafs[numleafs];
	numleafs++;

	leaf_p->contents = node->contents;

	VectorCopy(node->mins, leaf_p->mins);
	VectorCopy(node->maxs, leaf_p->maxs);

	leaf_p->visofs = -1;

	leaf_p->firstface = numfaces;

	for (f = node->markfaces; f; f++) {

		if (numfaces == MAX_MAP_FACES)
			Error("numfaces == MAX_MAP_FACES");

		df = &dfaces[numfaces++];

		df->numpoints = f->w->numpoints;
		dp = PlaneTodPlane(planes[f->planenum]);
		df->planenum = FindFinalPlane(&dp);
		df->texturenum = f->texturenum;
		df->points = malloc(3 * sizeof(df->numpoints));

		for (i = 0; i < df->points; i++)
			df->points = f->w->points[i];

	}

	leaf_p->numfaces = numfaces - leaf_p->firstface;
}

void WriteDrawNodes_r(node_t* node)
{
	dnode_t* n;
	int		i;

	if (numnodes == MAX_MAP_NODES)
		Error("numnodes == MAX_MAP_NODES");
	n = &dnodes[numnodes];
	numnodes++;

	VectorCopy(node->mins, n->mins);
	VectorCopy(node->maxs, n->maxs);

	n->planenum = node->outputplanenum;
	

	for (i = 0; i < 2; i++)
	{
		if (node->children[i]->planenum == -1)
		{
			if (node->children[i]->contents == CONTENTS_SOLID)
				n->children[i] = -1;
			else
			{
				n->children[i] = -(numleafs + 1);
				WriteLeaf(node->children[i]);
			}
		}
		else
		{
			n->children[i] = numnodes;
			WriteDrawNodes_r(node->children[i]);
		}
	}
}

void BeginBSPFile(void) {
	numleafs = 1;
	dleafs[0].contents = CONTENTS_SOLID;

	firstface = 0;
}


void WriteBSPFile() {

}