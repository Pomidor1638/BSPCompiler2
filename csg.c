#pragma once
#include "qbsp.h"

face_t* validfaces[MAX_PLANE_COUNT];

face_t* inside;
face_t* outside;

int csgfaces, brushfaces, csgmergefaces;

void SplitFace(face_t* f, plane_t* split, face_t** front, face_t** back) {
	face_t* f;
	winding_t *wf, *wb, *ws;

	*front = *back = NULL;

	ws = CopyWinding(f->w);

	DivideWinding(ws, split, &wf, &wb);

	if (wf) {
		*front = CopyFace(f);
		(*front)->next = NULL;
		FreeWinding((*front)->w);
		(*front)->w = wf;
		//if (!(*front)->origin)
		//	(*front)->origin = f;
	}

	if (wb) {
		*back = CopyFace(f);
		(*back)->next = NULL;
		FreeWinding((*back)->w);
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

		if (f->planenum == splitplane)
		{	// exactly on, handle special
			if (frontside != f->planeside || precedence)
			{	// allways clip off opposite faceing
				frags[frontside] = NULL;
				frags[!frontside] = f;
			}
			else
			{	// leave it on the outside
				frags[frontside] = f;
				frags[!frontside] = NULL;
			}
		}
		else {	// proper split
			SplitFace(f, split, &frags[0], &frags[1]);
		}


		//FCS: Simply add the face fragments in the outside or insidelist.

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



void CopyFacesToOutside(brush_t* b)
{
	face_t* f, * newf;

	outside = NULL;

	for (f = b->faces; f; f = f->next)
	{
		brushfaces++;

		newf = CopyFace(f);
		newf->next = outside;
		newf->contents[0] = CONTENTS_EMPTY;
		newf->contents[1] = b->contents;
		outside = newf;

	}
}



surface_t* CSGFaces(brushset_t* bs) {
	surface_t* surfhead;
	brush_t *b1, *b2;
	face_t* f;
	qbool overwrite;
	int i;

	memset(validfaces, 0, sizeof(validfaces));

	csgfaces = brushfaces = csgmergefaces = 0;
	

	for (b1 = bs->brushes; b1; b1 = b1->next) {

		CopyFacesToOutside(b1);

		overwrite = qfalse;

		for (b2 = bs->brushes; b2; b2 = b2->next) {
			
			if (b1 == b2) {
				overwrite = qtrue;
				continue;
			}

			for (i = 0; i < 3; i++)
				if (b1->mins[i] > b2->maxs[i] || b1->maxs[i] < b2->mins[i])
					break;
			if (i < 3)
				continue;

			inside = outside;
			outside = NULL;

			for (f = b2->faces; f; f = f->next) {
				ClipInside(f->planenum, f->planeside, overwrite);
			}


		}


	}

	return surfhead;
}