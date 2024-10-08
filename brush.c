#include "qbsp.h"


vec3_t mins, maxs;

int			numbrushplanes;
plane_t		planes[MAX_MAP_PLANES];

int			numbrushfaces;
mface_t		faces[128];

vec3_t	hull_size[3][2] = {
	{ {  0,  0,  0}, {  0,  0,  0} },
	{ {-16,-16,-32}, { 16, 16, 24} },
	{ {-32,-32,-64}, { 32, 32, 24} }
};

face_t* brush_faces;


void CreateBrushFaces(void) {

	int		 i, j, k;
	vec_t		   r;
	face_t*		   f;
	winding_t*     w;
	plane_t	   plane;
	mface_t*      mf;

	brush_faces = NULL;

	for (i = 0; i < numbrushfaces; i++) {

		mf = &faces[i];

		w = BaseWindingForPlane(&mf->plane);

		for (j = 0; j < numbrushfaces && w; j++) {

			if (i == j)
				continue;

			VectorSubtract(vec3_origin, faces[j].plane.normal, plane.normal);
			plane.dist = -faces[j].plane.dist;

			w = ClipWinding(w, &plane, qfalse);
		}

		if (!w)
			continue;

		f = AllocFace(w->numpoints);
		for (int i = 0; i < w->numpoints; i++)
			VectorCopy(w->points[i], f->w->points[i]);

		FreeWinding(w);

		


	}

}