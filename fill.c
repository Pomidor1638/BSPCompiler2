#include "qbsp.h"

int	outleafs;
int    valid;

node_t* PointInLeaf(node_t* node, vec3_t point) {
	vec_t	d;

	if (node->contents)
		return node;

	d = DotProduct(planes[node->planenum].normal, point) - planes[node->planenum].dist;

	if (d > 0)
		return PointInLeaf(node->children[0], point);

	return PointInLeaf(node->children[1], point);
}

qbool PlaceOccupant(int num, vec3_t point, node_t* headnode) {
	node_t* n;

	n = PointInLeaf(headnode, point);
	if (n->contents == CONTENTS_SOLID)
		return qfalse;
	n->occupied = num;
	return qtrue;
}

qbool RecursiveFillOutside(node_t* l, qbool fill) {
	portal_t* p;
	int			s;

	if (l->contents == CONTENTS_SOLID || l->contents == CONTENTS_SKY)
		return qfalse;

	if (l->valid == valid)
		return qfalse;

	if (l->occupied)
		return qtrue;

	l->valid = valid;

	// fill it and it's neighbors
	if (fill)
		l->contents = CONTENTS_SOLID;
	outleafs++;

	for (p = l->portals; p; )
	{
		s = (p->nodes[0] == l);

		if (RecursiveFillOutside(p->nodes[s], fill)) {	
			// leaked, so stop filling
			/*if (backdraw-- > 0)
			{
				MarkLeakTrail(p);
				DrawLeaf(l, 2);
			}*/
			return qtrue;
		}
		p = p->next[!s];
	}

	return qfalse;
}

void ClearOutFaces(node_t* node) {
	face_t** fp;

	if (node->planenum != -1) {
		ClearOutFaces(node->children[0]);
		ClearOutFaces(node->children[1]);
		return;
	}
	if (node->contents != CONTENTS_SOLID)
		return;

	for (fp = node->markfaces; *fp; fp++) {
		// mark all the original faces that are removed
		(*fp)->marked = qtrue;
	}
	node->faces = NULL;
}

qbool FillOutside(node_t* node)
{
	int			s;
	vec_t* v;
	int			i;
	qbool	inside;

	printf("*============* FillOutside *=============*\n");

	if (nofill)
	{
		printf("skipped\n");
		return qfalse;
	}

	inside = qfalse;
	for (i = 1; i < num_entities; i++)
	{
		if (!VectorCompare(entities[i].origin, vec3_origin))
		{
			if (PlaceOccupant(i, entities[i].origin, node))
				inside = qtrue;
		}
	}

	if (!inside)
	{
		printf("Hullnum %i: No entities in empty space -- no filling performed\n", hullnum);
		return qfalse;
	}

	s = !(outside_node.portals->nodes[1] == &outside_node);

	// first check to see if an occupied leaf is hit
	outleafs = 0;
	valid++;

	/*prevleaknode = NULL;*/

	/*if (!hullnum)
	{
		leakfile = fopen(pointfilename, "w");
		if (!leakfile)
			Error("Couldn't open %s\n", pointfilename);
	}*/

	if (RecursiveFillOutside(outside_node.portals->nodes[s], qfalse))
	{
		v = entities[0].origin;
		printf("leaked\n");
		qprintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		qprintf("reached occupant at: (%4.0f,%4.0f,%4.0f)\n"
			, v[0], v[1], v[2]);
		qprintf("no filling performed\n");
		//if (!hullnum)
		//	fclose(leakfile);
		//qprintf("leak file written to %s\n", pointfilename);
		qprintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		return qfalse;
	}
	/*if (!hullnum)
		fclose(leakfile);*/

	printf("filled\n");
	// now go back and fill things in
	valid++;
	RecursiveFillOutside(outside_node.portals->nodes[s], qtrue);

	// remove faces from filled in leafs	
	ClearOutFaces(node);

	qprintf("%4i outleafs\n", outleafs);
	return qtrue;
}