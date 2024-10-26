#include "qbsp.h"

void ShowWinding(winding_t* w) {
	for (int i = 0; i < w->numpoints; i++) {
		printf("%f, %f, %f\n", w->points[i][0], w->points[i][1], w->points[i][2]);
	}
	printf("\n");
}

void ShowBrush(brush_t* b) {
	int i = 0;
	plane_t p;
	for (face_t* f = b->faces; f; f = f->next, i++) {
		p = planes[f->planenum];
		if (f->planeside) {
			VectorSubtract(vec3_origin, p.normal, p.normal);
			p.dist = -p.dist;
		}

		printf("face %i\nplane: %f, %f, %f, %f\n ", i, p.normal[0], p.normal[1], p.normal[2], p.dist);
		ShowWinding(f->w);
		printf("\n");
	}
	printf("\n");
}


void ShowSurface(surface_t* surf) {
	surface_t* s;
	face_t*    f;

	for (s = surf; s; s = s->next) {
		for (f = s->faces; f; f = f->next) {
			ShowWinding(f->w);
		}
		printf("\n");
	}
	printf("\n");
}

brush_t* CreateBox(vec3_t origin, vec3_t sizes) {

	mface_t faces[6];
	mbrush_t mb;
	brush_t* b;
	int i;

	mb.contents = CONTENTS_SOLID;

	memset(&faces[0], 0, sizeof(faces));

	
	for (i = 0; i < 3; i++) {

		faces[i].plane.normal[i] = 1;
		faces[5 - i].plane.normal[i] = -1;

		faces[i].plane.dist    =   sizes[i] + origin[i];
		faces[5 - i].plane.dist =  sizes[i] - origin[i];
	}

	for (int i = 0; i < 5; i++) {
		faces[i].next = &faces[i + 1];
	}


	mb.faces = &faces[0];
	mb.next = NULL;


	b = LoadBrush(&mb, 0);

	/*vec3_t p;

	for (int i = 0; i < 6; i++) {
		plane_t pl = faces[i].plane;
		VectorCopy(pl.normal, p);
		printf("plane %i: %f, %f, %f, %f\n", i, p[0], p[1], p[2], pl.dist);
	}*/

	return b;
}


int main() {

	brush_t* b;
	surface_t *surf, *next, *s;

	vec3_t origin = {  0,  0,  0 };
	vec3_t sizes  = { 10, 10, 10 };


	b = CreateBox(origin, sizes);


	origin[1] += 20;
	origin[2] += 20;

	b->next = CreateBox(origin, sizes);

	brushset_t* bs;

	bs = AllocBrushset();
	bs->brushes = b;

	surf = CSGFaces(bs);

	for (s = surf; s; s = next) {
		next = s->next;
		FreeSurf(s, qtrue);
	}

	FreeBrushset(bs);

	PrintMemory();

	return 0;
}
