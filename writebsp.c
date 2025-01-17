#include "qbsp.h"

int		headclipnode;

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

int FindFinalVertex(vec3_t v) {
	dvertex_t dv;
	float* dvertex;
	int i, j;
	VectorCopy(v, dv.v);

	for (i = 0; i < numvert; i++) {
		dvertex = dvertexes[i].v;
		for (j = 0; j < 3; j++)
			if (dvertexes[i].v[j] != dv.v[j])
				break;
		if (j == 3)
			return i;
	}

	if (numvert == MAX_MAP_VERTS)
		Error("vertnum == MAX_MAP_VERTS");

	dvertexes[i] = dv;

	return numvert++;
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
		return num;
	}

	if (numclipnodes == MAX_MAP_CLIPNODES)
		Error("numclipnodes == MAX_MAP_CLIPNODES");
	// emit a clipnode
	c = numclipnodes;
	cn = &dclipnodes[numclipnodes];
	numclipnodes++;
	cn->planenum = node->outputplanenum;
	for (i = 0; i < 2; i++) 
		cn->children[i] = WriteClipNodes_r(node->children[i]);
	return c;
}

void WriteClipNodes(node_t* nodes) {

	headclipnode = numclipnodes;
	WriteClipNodes_r(nodes);
}

void BumpModel(int hullnum) {
	dmodel_t* bm;

	if (nummodels == MAX_MAP_MODELS)
		Error("nummodels == MAX_MAP_MODELS");
	bm = &dmodels[nummodels];
	nummodels++;

	bm->headnode[hullnum] = headclipnode;
}

void WriteLeaf(node_t* node) {

	int i, side;
	face_t **f;
	portal_t* prt;
	dleaf_t* leaf_p;
	dface_t* df;
	dplane_t dp;
	dportal_t* dprt;

	leaf_p = &dleafs[numleafs];
	node->outputleafnum = numleafs;
	numleafs++;

	leaf_p->contents = node->contents;

	VectorCopy(node->mins, leaf_p->mins);
	VectorCopy(node->maxs, leaf_p->maxs);

	//leaf_p->visofs = -1;

	leaf_p->firstface = numfaces;

	for (f = node->markfaces; *f; f++) {

		if (numfaces == MAX_MAP_FACES)
			Error("numfaces == MAX_MAP_FACES");

		df = &dfaces[numfaces++];

		dp = PlaneTodPlane(planes[(*f)->planenum]);
		df->planenum = FindFinalPlane(&dp);
		df->side = (*f)->planeside;
		df->texturenum = FindFinalTexinfo(&texinfo[(*f)->texturenum]);

		df->firstpoint = numvertextable;
		df->numpoints = (*f)->w->numpoints;



		for (i = 0; i < df->numpoints; i++) {
			if (numvertextable == MAX_MAP_VERTEXTABLE)
				Error("numvertextable == MAX_MAP_VERTEXTABLE");

			dvertextable[numvertextable] = FindFinalVertex((*f)->w->points[i]);

			numvertextable++;
		}

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

	for (i = 0; i < 2; i++) {
		if (node->children[i]->planenum == -1) {
			if (node->children[i]->contents == CONTENTS_SOLID)
				n->children[i] = -1;
			else {
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

void WriteDrawNodes(node_t* headnode) {

	int		i;
	int		start;
	dmodel_t* bm;

	if (nummodels == MAX_MAP_MODELS)
		Error("nummodels == MAX_MAP_MODELS");

	bm = &dmodels[nummodels];
	nummodels++;

	bm->headnode[0] = numnodes;
	bm->firstface = numfaces;

	start = numleafs;

	if (headnode->contents < 0)
		WriteLeaf(headnode);
	else
		WriteDrawNodes_r(headnode);

	bm->numfaces = numfaces - bm->firstface;
	bm->visleafs = numleafs - start;

	for (i = 0; i < 3; i++) {
		bm->mins[i] = headnode->mins[i];	
		bm->maxs[i] = headnode->maxs[i];
	}
}

