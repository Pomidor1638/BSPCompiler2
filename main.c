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

mbrush_t* CreateBox(vec3_t origin, vec3_t sizes, int contents, int hullnum) {

	mface_t faces[6];
	mbrush_t mb;
	brush_t* b;
	int i;

	mb.contents = contents;

	memset(&faces[0], 0, sizeof(faces));

	
	for (i = 0; i < 3; i++) {

		faces[i].plane.normal[i] = 1;
		faces[5 - i].plane.normal[i] = -1;

		faces[i].plane.dist     =  sizes[i] + origin[i];
		faces[5 - i].plane.dist =  sizes[i] - origin[i];
	}

	for (int i = 0; i < 5; i++) {
		faces[i].next = &faces[i + 1];
	}


	mb.faces = &faces[0];
	mb.next = NULL;

	return 0;
}

void AddBoxToEntity(model_t* ent, vec3_t origin, vec3_t sizes, int contents) {

}


int main() {

	return 0;

}
