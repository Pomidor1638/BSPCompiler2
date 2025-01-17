#pragma once
#include "qbsp.h"

face_t* validfaces[MAX_MAP_PLANES];

face_t* inside;
face_t* outside;

int csgfaces, brushfaces, csgmergefaces;

void SplitFace(face_t* f, plane_t* split, face_t** front, face_t** back) {

	winding_t *wf, *wb, *ws;

	*front = *back = NULL;
	
	ws = CopyWinding(f->w);

	DivideWinding(ws, split, &wf, &wb);

	if (wf) {
		*front = ExtendFace(f);
		(*front)->w = wf;
	}

	if (wb) {
		*back = ExtendFace(f);
		(*back)->w = wb;
	}

	FreeFace(f);
}


void ClipInside(int splitplane, int frontside, qbool precedence) {
	face_t* f, * next;
	face_t* frags[2];
	face_t* insidelist;  
	plane_t* split;

	split = &planes[splitplane];

	insidelist = NULL;

	for (f = inside; f; f = next)
	{
		next = f->next;

		if (f->planenum == splitplane) {	
			if (frontside != f->planeside || precedence) {	
				frags[frontside] = NULL;
				frags[!frontside] = f;
			}
			else {	
				frags[frontside] = f;
				frags[!frontside] = NULL;
			}
		}
		else {
			SplitFace(f, split, &frags[0], &frags[1]);
		}

		if (frags[frontside]) {
			frags[frontside]->next = outside;
			outside = frags[frontside];
		}

		if (frags[!frontside]) {
			frags[!frontside]->next = insidelist;
			insidelist = frags[!frontside];
		}


	}

	inside = insidelist;
}

void SaveOutside(qbool mirror)
{
	face_t* f, * next, * newf;
	int		i;
	int		planenum;

	for (f = outside; f; f = next) {
		next = f->next;
		csgfaces++;

		planenum = f->planenum;

		if (mirror) {
			
			newf = CopyFace(f);

			newf->planeside = !(f->planeside);	
			newf->contents[0] = f->contents[1];
			newf->contents[1] = f->contents[0];

			for (i = 0; i < f->w->numpoints; i++) {
				VectorCopy(f->w->points[f->w->numpoints - 1 - i], newf->w->points[i]);
			}
			
		}
		else
			newf = NULL;

		f->next = validfaces[planenum];
		validfaces[planenum] = f;

		if (newf) {
			newf->next = validfaces[planenum];
			validfaces[planenum] = newf;
		}

	}
}

void FreeInside(int contents)
{
	face_t *f, *next;

	for (f = inside; f; f = next) {
		next = f->next;

		if (contents != CONTENTS_SOLID) {
			f->contents[0] = contents;
			f->next = outside;
			outside = f;
		}
		else
			FreeFace(f);
	}
}

void CopyFacesToOutside(brush_t* b)
{
	face_t *f, *newf;

	outside = NULL;

	for (f = b->faces; f; f = f->next) {
		
		brushfaces++;
		newf = CopyFace(f);
		newf->next = outside;
		newf->contents[0] = CONTENTS_EMPTY;
		newf->contents[1] = b->contents;
		outside = newf;
	}
}

void CalcSurfaceInfo(surface_t* surf) {
	face_t *f;
	int  i, j, num;
	vec3_t  p;

	qprintf("*-----------* CalcSurfaceInfo *----------*\n");

	for (i = 0; i < 3; i++) {
		surf->mins[i] =  MAX_RANGE;
		surf->maxs[i] = -MAX_RANGE;
	}
	num = 0;
	for (f = surf->faces; f; f = f->next) {
		
		if (f->contents[0] >= 0 || f->contents[1] >= 0)
			Error("CalcSurfInfo: Bad Contents");

		for (i = 0; i < f->w->numpoints; i++) {

			VectorCopy(f->w->points[i], p);

			for (j = 0; j < 3; j++) {
				if (p[j] < surf->mins[j])
					surf->mins[j] = p[j];
				if (p[j] > surf->maxs[j])
					surf->maxs[j] = p[j];
			}
		}
		num++;
	}

	VectorCopy(surf->mins, p);
	VectorCopy(surf->maxs, p);
	qprintf("%5i faces\n", num);

}


surface_t* BuildSurfaces(void)
{
	face_t          **f;
	face_t       *count;
	int               i;
	surface_t        *s;
	surface_t *surfhead;

	surfhead = NULL;

	f = validfaces;
	for (i = 0; i < numbrushplanes; i++, f++)
	{
		if (!*f)
			continue;	

		s = AllocSurface();
		s->planenum = i;          
		s->next = surfhead;	

		surfhead = s;
		s->faces = *f;
		for (count = s->faces; count; count = count->next)
			csgmergefaces++;
		CalcSurfaceInfo(s);	
	}

	return surfhead;
}


surface_t* CSGFaces(brushset_t* bs) {
	brush_t* b1, * b2;
	int			i;
	qbool		overwrite;
	face_t* f;
	surface_t* surfhead;

	printf("*==============* CSGFaces *==============*\n");

	memset(validfaces, 0, sizeof(validfaces));

	csgfaces = brushfaces = csgmergefaces = 0;

	int n = 0;

	for (b1 = bs->brushes; b1; b1 = b1->next) {
		
		CopyFacesToOutside(b1);

		overwrite = 0;

		for (b2 = bs->brushes; b2; b2 = b2->next) {

			if (b1 == b2) {
				overwrite = 1;	
				continue;
			}

			for (i = 0; i < 3; i++)
				if (b1->mins[i] > b2->maxs[i] || b1->maxs[i] < b2->mins[i])
					break;
			if (i < 3) 
				continue;


			inside = outside;
			outside = NULL;

			for (f = b2->faces; f; f = f->next)
				ClipInside(f->planenum, f->planeside, overwrite);

			if (b1->contents == CONTENTS_SOLID && b2->contents <= CONTENTS_WATER)
				FreeInside(b2->contents);
			else
				FreeInside(CONTENTS_SOLID);
		}

		if (b1->contents != CONTENTS_SOLID)
			SaveOutside(qtrue);	
		else
			SaveOutside(qfalse);

		n++;
		qprintf("%5i brush finished\n", n);
	}

	surfhead = BuildSurfaces();


	qprintf("*==========* CSGFaces finished *=========*\n");
	printf("%5i brushfaces\n", brushfaces);
	printf("%5i csgfaces\n", csgfaces);
	printf("%5i mergedfaces\n", csgmergefaces);

	return surfhead;
}

int deletedfaces;

void GatherNodeFaces_r(node_t* node) {
	face_t* f, * next;

	if (node->planenum != PLANENUM_LEAF) {
		for (f = node->faces; f; f = next) {
			next = f->next;
			if (f->marked) {	
				FreeFace(f);
				deletedfaces++;
			}
			else {
				f->next = validfaces[f->planenum];
				validfaces[f->planenum] = f;
			}
		}

		GatherNodeFaces_r(node->children[0]);
		GatherNodeFaces_r(node->children[1]);

		free(node);
	}
	else {
		free(node);
	}
}

surface_t* GatherNodeFaces(node_t* headnode) {
	qprintf("*===========* GatherNodeFaces *===========*\n");
	memset(validfaces, 0, sizeof(validfaces));
	GatherNodeFaces_r(headnode);
	qprintf("%5i faces deleted\n", deletedfaces);
	return BuildSurfaces();
}