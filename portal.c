#include "qbsp.h"


node_t	outside_node;		

void PrintPortal(portal_t* p) {
	int			i;
	winding_t* w;

	w = p->winding;
	for (i = 0; i < w->numpoints; i++)
		printf("(%5.0f,%5.0f,%5.0f)\n", w->points[i][0]
			, w->points[i][1], w->points[i][2]);
}

void AddPortalToNodes(portal_t* p, node_t* front, node_t* back) {
	if (p->nodes[0] || p->nodes[1])
		Error("AddPortalToNode: allready included");

	p->nodes[0] = front;
	p->next[0] = front->portals;
	front->portals = p;

	p->nodes[1] = back;
	p->next[1] = back->portals;
	back->portals = p;
}

void RemovePortalFromNode(portal_t* portal, node_t* l) {
	portal_t** pp, * t;

	pp = &l->portals;
	while (1)
	{
		t = *pp;
		if (!t)
			Error("RemovePortalFromNode: portal not in leaf");

		if (t == portal)
			break;

		if (t->nodes[0] == l)
			pp = &t->next[0];
		else if (t->nodes[1] == l)
			pp = &t->next[1];
		else
			Error("RemovePortalFromNode: portal not bounding leaf");
	}

	if (portal->nodes[0] == l) {
		*pp = portal->next[0];
		portal->nodes[0] = NULL;
	}
	else if (portal->nodes[1] == l) {
		*pp = portal->next[1];
		portal->nodes[1] = NULL;
	}
}

void MakeHeadnodePortals(node_t* node)
{
	vec3_t		bounds[2];
	int			i, j, n;
	portal_t* p, * portals[6];
	plane_t		bplanes[6], * pl;
	int			side;


	// pad with some space so there will never be null volume leafs
	for (i = 0; i < 3; i++)
	{
		bounds[0][i] = brushset->mins[i] - SIDESPACE;
		bounds[1][i] = brushset->maxs[i] + SIDESPACE;
	}

	outside_node.contents = CONTENTS_SOLID;
	outside_node.portals = NULL;
	outside_node.outputleafnum = 0;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 2; j++)
		{
			n = j * 3 + i;

			p = AllocPortal();
			portals[n] = p;

			pl = &bplanes[n];
			memset(pl, 0, sizeof(*pl));
			if (j)
			{
				pl->normal[i] = -1;
				pl->dist = -bounds[j][i];
			}
			else
			{
				pl->normal[i] = 1;
				pl->dist = bounds[j][i];
			}
			p->planenum = FindPlane(pl, &side);

			p->winding = BaseWindingForPlane(pl);
			if (side)
				AddPortalToNodes(p, &outside_node, node);
			else
				AddPortalToNodes(p, node, &outside_node);
		}

	// clip the basewindings by all the other planes
	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 6; j++)
		{
			if (j == i)
				continue;
			portals[i]->winding = ClipWinding(portals[i]->winding, &bplanes[j], qtrue);
		}
	}
}


void CutNodePortals_r(node_t* node)
{
	plane_t* plane, clipplane;
	node_t* f, * b, * other_node;
	portal_t* p, * new_portal, * next_portal;
	winding_t* w, * frontwinding, * backwinding;
	int			side;

	
	if (node->contents) 
		return;			
	

	plane = &planes[node->planenum];

	f = node->children[0];
	b = node->children[1];
	
	new_portal = AllocPortal();
	new_portal->planenum = node->planenum;

	w = BaseWindingForPlane(&planes[node->planenum]);

	side = 0;	
	for (p = node->portals; p; p = p->next[side]) {
		clipplane = planes[p->planenum];
		if (p->nodes[0] == node)
			side = 0;
		else if (p->nodes[1] == node) {
			clipplane.dist = -clipplane.dist;
			VectorSubtract(vec3_origin, clipplane.normal, clipplane.normal);
			side = 1;
		}
		else
			Error("CutNodePortals_r: mislinked portal");

		w = ClipWinding(w, &clipplane, qtrue);
		if (!w) {
			printf("WARNING: CutNodePortals_r:new portal was clipped away\n");
			break;
		}
	}

	if (w) {
		new_portal->winding = w;
		AddPortalToNodes(new_portal, f, b);
	}

	for (p = node->portals; p; p = next_portal)
	{
		if (p->nodes[0] == node)
			side = 0;
		else if (p->nodes[1] == node)
			side = 1;
		else
			Error("CutNodePortals_r: mislinked portal");
		next_portal = p->next[side];

		other_node = p->nodes[!side];
		RemovePortalFromNode(p, p->nodes[0]);
		RemovePortalFromNode(p, p->nodes[1]);

		DivideWinding(p->winding, plane, &frontwinding, &backwinding);

		if (!frontwinding) {
			if (side == 0)
				AddPortalToNodes(p, b, other_node);
			else
				AddPortalToNodes(p, other_node, b);
			continue;
		}
		if (!backwinding) {
			if (side == 0)
				AddPortalToNodes(p, f, other_node);
			else
				AddPortalToNodes(p, other_node, f);
			continue;
		}

		// the winding is split
		new_portal = AllocPortal();
		*new_portal = *p;
		new_portal->winding = backwinding;
		p->winding = frontwinding;

		if (side == 0) {
			AddPortalToNodes(p, f, other_node);
			AddPortalToNodes(new_portal, b, other_node);
		}
		else {
			AddPortalToNodes(p, other_node, f);
			AddPortalToNodes(new_portal, other_node, b);
		}
	}

	CutNodePortals_r(f);
	CutNodePortals_r(b);

}


void PortalizeWorld(node_t* headnode) {
	qprintf("*=============* portalize *==============*\n");

	MakeHeadnodePortals(headnode);
	CutNodePortals_r(headnode);
}

void FreeAllPortals(node_t* node)
{
	portal_t* p, * nextp;

	if (!node->contents) {
		FreeAllPortals(node->children[0]);
		FreeAllPortals(node->children[1]);
	}

	for (p = node->portals; p; p = nextp) {
		if (p->nodes[0] == node)
			nextp = p->next[0];
		else
			nextp = p->next[1];
		RemovePortalFromNode(p, p->nodes[0]);
		RemovePortalFromNode(p, p->nodes[1]);
		FreeWinding(p->winding);
		FreePortal(p);
	}
}

int FindPortal(dportal_t* portal) {
	int i;
	dportal_t* portals;

	if (!portal) 
		Error("FindPortal: bad pointer");
	
	if (portal->leafs[0] == -1 || portal->leafs[1] == -1)
		Error("FindPortal: mislinked portal");

	portals = dportals;

	for (i = 0; i < numportals; i++) {
		if (memcmp(portal, portals, sizeof(dportal_t)) == 0)
			return i;
		portals++;
	}

	if (numportals == MAX_MAP_PORTALS)
		Error("FindPortal: numportals == MAX_MAP_PORTALS");

	*portals = *portal;
	numportals++;
	
	return i;
}

void WritePortals_r(node_t* node) {

	int		   i, side;
	dportal_t     dprt;
	dleaf_t*     dleaf;
	node_t* other_node;
	portal_t*      prt;
	winding_t*       w;

	if (!node->contents) {
		if (node->portals)
			Error("WritePortals_r: portal in decision node");
		WritePortals_r(node->children[0]);
		WritePortals_r(node->children[1]);
		return;
	}

	if (node->contents == CONTENTS_SOLID) 
		return;
	
	dleaf = &dleafs[node->outputleafnum];
	dleaf->firstportal = numportaltable;

	side = 0;
	for (prt = node->portals; prt; prt = prt->next[side]) {

		if (prt->nodes[0] == node)
			side = 0;
		else if (prt->nodes[1] == node)
			side = 1;
		else
			Error("WritePortals_r: mislinked portal");

		 other_node = prt->nodes[!side];

		 if ((other_node->contents == CONTENTS_SOLID || node->contents != other_node->contents) && other_node != &outside_node)
			 continue;

		dprt.planenum     = prt->planenum;
		dprt.leafs[ side] = node->outputleafnum;
		dprt.leafs[!side] = other_node->outputleafnum;
		w = prt->winding;

		dprt.firstpoint = numvertextable;
		dprt.numpoints = prt->winding->numpoints;
		for (i = 0; i < w->numpoints; i++) {
			if (numvertextable == MAX_MAP_VERTEXTABLE)
				Error("WritePortals_r: numvertextable == MAX_MAP_VERTEXTABLE");
			dvertextable[numvertextable] = FindFinalVertex(w->points[i]);
			numvertextable++;
		}
		dportaltable[numportaltable] = FindPortal(&dprt);
		numportaltable++;
	}
	dleaf->numportals = numportals - dleaf->firstportal;
}

void WritePortals(node_t* headnode) {
	numportals = 0;
	WritePortals_r(headnode);
}