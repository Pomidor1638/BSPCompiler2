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
	face_t* f;

	for (s = surf; s; s = s->next) {
		for (f = s->faces; f; f = f->next) {
			ShowWinding(f->w);
		}
		printf("\n");
	}
	printf("\n");
}

void UpdateFaces(mface_t* f, double scale) {
	vec3_t xv, yv;
	for (; f; f = f->next) {
		TextureAxisFromPlane(&f->plane, xv, yv);
		VectorMA(vec3_origin, scale, xv, f->texinfo.vecs[0]);
		VectorMA(vec3_origin, scale, yv, f->texinfo.vecs[1]);
		f->texinfo.vecs[0][3] = 0;
		f->texinfo.vecs[1][3] = 0;
	}
}

mbrush_t* CreateBox(vec3_t origin, vec3_t sizes, int contents, int hullnum, int textnum, double scale) {

	mface_t* faces;
	mface_t* f1, * f2;
	mbrush_t* mb;
	vec3_t xv, yv;
	int i, j;

	mb = malloc(sizeof(mbrush_t));
	memset(mb, 0, sizeof(mbrush_t));

	mb->contents = contents;

	faces = malloc(6 * sizeof(mface_t));
	memset(&faces[0], 0, 6 * sizeof(mface_t));

	for (i = 0; i < 3; i++) {
		f1 = &faces[i];
		f2 = &faces[5 - i];

		f1->plane.normal[i] = 1;
		f2->plane.normal[i] = -1;

		f1->plane.dist = sizes[i] + origin[i];
		f2->plane.dist = sizes[i] - origin[i];

		f1->texinfo.texturenum = f2->texinfo.texturenum = textnum;

	}

	for (i = 0; i < 5; i++)
		faces[i].next = &faces[i + 1];

	UpdateFaces(faces, scale);


	mb->faces = &faces[0];
	mb->next = NULL;


	return mb;
}

void AddBoxToEntity(entity_t* ent, int contents, int textnum, double scale, vec_t x, vec_t y, vec_t z, vec_t a, vec_t b, vec_t c) {
	mbrush_t* box, ** mb;
	vec3_t origin = { x, y, z };
	vec3_t sizes = { a, b, c };

	box = CreateBox(origin, sizes, contents, hullnum, textnum, scale);

	for (mb = &ent->brushes; *mb; mb = &(*mb)->next)
	{
	}

	*mb = box;
}

void Rotate(vec3_t v, vec_t angle, int axis) {
	double sin_a;
	double cos_a;

	double a, b;
	int x0, x1;


	switch (axis)
	{
	case 0:
		x0 = 1;
		x1 = 2;
		break;
	case 1:
		x0 = 0;
		x1 = 2;
		break;
	case 2:
		x0 = 0;
		x1 = 1;
		break;
	default:
		return;
	}

	sin_a = sin(angle);
	cos_a = cos(angle);

	a = v[x0];
	b = v[x1];

	v[x0] = a * cos_a + b * sin_a;
	v[x1] = -a * sin_a + b * cos_a;
}



int main() {

	system("chcp 1251>nul");


	hullnum = 0;
	verbose = qfalse;

	planecount = &numbrushplanes;

	int i;
	entity_t* ent;
	face_t* f;
	mface_t* mf;
	mbrush_t* mb;
	surface_t* surf, * s;

	BeginBSPFile();

	ent = malloc(sizeof(entity_t));
	memset(ent, 0, sizeof(entity_t));

	vec3_t origin;

	vec3_t sizes;

	srand(time(NULL));

	/*for (int i = 1; i <= 100; i++) {

		AddBoxToEntity(ent, CONTENTS_SOLID, (i % 143 + 1) == 2 ? 1 : (i % 143 + 1), 0.005,
			(rand() % 256) * 16 - 256*8, 
			(rand() % 256) * 16 - 256*8, 
			(rand() % 256) * 16 - 256*8,

			rand() % 128 + 32, 
			rand() % 128 + 32, 
			rand() % 128 + 32
		);
	}
	

	for (mb = ent->brushes; mb; mb = mb->next) {
		int x = (rand() % 360);
		int y = (rand() % 360);
		int z = (rand() % 360);
		for (mf = mb->faces; mf; mf = mf->next) {
			Rotate(mf->plane.normal, Q_PI * (x) / 180.0, 0);
			Rotate(mf->plane.normal, Q_PI * (y) / 180.0, 1);
			Rotate(mf->plane.normal, Q_PI * (z) / 180.0, 2);
		}
		UpdateFaces(mb->faces, (rand() % 18 + 2) * 0.001);
	}*/

	AddBoxToEntity(ent, CONTENTS_SOLID, 14, 0.005, 0, 500, 0, 200, 100, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 15, 0.005, 0, -500, 0, 400, 100, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 16, 0.005, 500, 0, 0, 100, 500, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 17, 0.005, -500, 0, 0, 100, 500, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 16, 0.005, 0, 0, 500, 600, 600, 100);
	AddBoxToEntity(ent, CONTENTS_SOLID, 17, 0.005, 0, 0, -500, 600, 600, 100);

	AddBoxToEntity(ent, CONTENTS_SOLID, 14, 0.005, 2000, 400, 0, 400, 100, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 15, 0.005, 2000, -400, 0, 400, 100, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 16, 0.005, 2500, 0, 0, 100, 500, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 17, 0.005, 1500, 0, 0, 100, 500, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 16, 0.005, 2000, 0, 500, 600, 600, 100);
	AddBoxToEntity(ent, CONTENTS_SOLID, 17, 0.005, 2000, 0, -500, 600, 600, 100);

	AddBoxToEntity(ent, CONTENTS_SOLID, 14, 0.005, -1500, 400, 0, 400, 100, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 15, 0.005, -1500, -400, 0, 400, 100, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 16, 0.005, -1000, 0, 0, 100, 500, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 17, 0.005, -2000, 0, 0, 100, 500, 500);
	AddBoxToEntity(ent, CONTENTS_SOLID, 16, 0.005, -1500, 0, 500, 600, 600, 100);
	AddBoxToEntity(ent, CONTENTS_SOLID, 17, 0.005, -1500, 0, -500, 600, 600, 100);

	
	AddBoxToEntity(ent, CONTENTS_WATER, 135, 0.01, 100, 100, 100, 100, 100, 100);


	AddBoxToEntity(ent, CONTENTS_SOLID, 13, 0.01,  1000,  1000,  1000, 100, 100, 100);
	AddBoxToEntity(ent, CONTENTS_SOLID, 13, 0.01,  1100,  1000, -1000, 100, 100, 100);
	AddBoxToEntity(ent, CONTENTS_SOLID, 13, 0.01,  1000, -1100,  1000, 100, 100, 100);
	AddBoxToEntity(ent, CONTENTS_SOLID, 13, 0.01,  1000, -1000, -1000, 100, 100, 100);
												  
	AddBoxToEntity(ent, CONTENTS_SOLID, 13, 0.01, -1100,  1000,  1000, 100, 100, 100);
	AddBoxToEntity(ent, CONTENTS_SOLID, 13, 0.01, -1000,  1000, -1100, 100, 100, 100);
	AddBoxToEntity(ent, CONTENTS_SOLID, 13, 0.01, -1100, -1000,  1000, 100, 100, 100);
	AddBoxToEntity(ent, CONTENTS_SOLID, 13, 0.01, -1000, -1000, -1000, 100, 100, 100);

	num_entities = 2;
	entities[0] = *ent;
	entities[1].origin[0] = 1;
	entities[1].origin[1] = 1;
	entities[1].origin[2] = 1;

	//======================================================================================================

	nummodels = 0;
	hullnum = 0;

	brushset = Brush_LoadEntity(ent, hullnum);
	surf = CSGFaces(brushset);


	node_t* node = SolidBSP(surf, qtrue, qfalse);

	PortalizeWorld(node);

	if (FillOutside(node)) {

		FreeAllPortals(node);
		surf = GatherNodeFaces(node);
		node = SolidBSP(surf, qfalse, qtrue);
		PortalizeWorld(node);

	}
	else {
		FreeAllPortals(node);
		FreeNode_r(node);
		surf = CSGFaces(brushset);
		node = SolidBSP(surf, qfalse, qtrue);
		PortalizeWorld(node);
	}


	WriteNodePlanes(node);
	WriteDrawNodes(node);
	WritePortals(node);
	FreeAllPortals(node);
	FreeNode_r(node);
	FreeBrushset(brushset);

	//=================================================================================================

	nummodels = 0;
	hullnum = 1;

	brushset = Brush_LoadEntity(ent, hullnum);

	surf = CSGFaces(brushset);


	node = SolidBSP(surf, qtrue, qfalse);

	PortalizeWorld(node);

	if (FillOutside(node)) {

		FreeAllPortals(node);
		surf = GatherNodeFaces(node);
		node = SolidBSP(surf, qfalse, qtrue);

	}
	else {
		FreeAllPortals(node);
		FreeNode_r(node);
		surf = CSGFaces(brushset);
		node = SolidBSP(surf, qfalse, qtrue);
	}

	WriteNodePlanes(node);
	WriteClipNodes(node);
	FreeNode_r(node);
	BumpModel(hullnum);

	printf("\n");


	char* name = "D:/Великая папка/Программы/BSPENG/ogl_begin/test.bsp";

	for (i = 0; name[i]; i++)
		bspfilename[i] = name[i];

	bspfilename[i] = '\0';

	FinishBSPFile();

	return 0;

}
