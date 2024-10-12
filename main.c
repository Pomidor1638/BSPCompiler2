#include "qbsp.h"

void ShowWinding(winding_t* w) {
	for (int i = 0; i < w->numpoints; i++) {
		printf("%f, %f, %f\n", w->points[i][0], w->points[i][1], w->points[i][2]);
	}
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
}


int main() {

	mbrush_t mb;
	brush_t*  b;
	mb.next = NULL;

	plane_t planes_buf[6] = {
		{{ 0, 1, 1}, 1},
		{{ 0, 1, 0}, 1},
		{{ 1, 0, 0}, 1},
		{{ 0, 0,-1}, 1},
		{{ 0,-1, 0}, 1},
		{{-1, 0, 0}, 1}
	};

	VectorNormalize(planes_buf[0].normal);

	mface_t faces[6];

	for (int i = 0; i < 6; i++) {
		//printf("%i\n", i);
		faces[i].next = NULL;
		faces[i].plane = planes_buf[i];
		faces[i].texinfo = 0;
	}

	for (int i = 0; i < 5; i++) 
	{
		//printf("%p\n", &faces[i + 1]);
		faces[i].next = &faces[i + 1];
	}

	mb.faces = &faces[0];
	//mb.faces->next = NULL;

	

	b = LoadBrush(&mb, 2);
	ShowBrush(b);



	return 0;
}
