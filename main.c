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

void UpdateFaces(mface_t* f) {
	vec3_t xv, yv;
	for (; f; f = f->next) {
		TextureAxisFromPlane(&f->plane, xv, yv);
		VectorMA(vec3_origin, 0.008, xv, f->texinfo.vecs[0]);
		VectorMA(vec3_origin, 0.008, yv, f->texinfo.vecs[1]);
		f->texinfo.vecs[0][3] = 0;
		f->texinfo.vecs[1][3] = 0;
	}
}

mbrush_t* CreateBox(vec3_t origin, vec3_t sizes, int contents, int hullnum, int textnum) {

	mface_t* faces;
	mface_t* f1,*f2;
	mbrush_t *mb;
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

		f1->plane.normal[i] =  1;
		f2->plane.normal[i] = -1;

		f1->plane.dist =  sizes[i] + origin[i];
		f2->plane.dist =  sizes[i] - origin[i];

		f1->texinfo.texturenum = f2->texinfo.texturenum = textnum;

	}

	for (i = 0; i < 5; i++)
		faces[i].next = &faces[i + 1];

	UpdateFaces(faces);


	mb->faces = &faces[0];
	mb->next = NULL;
	

	return mb;
}

int hullnum;

void AddBoxToEntity(entity_t* ent, vec3_t origin, vec3_t sizes, int contents, int textnum) {
	mbrush_t *box, **mb;

	box = CreateBox(origin, sizes, contents, hullnum, textnum);

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

	v[x0] =  a * cos_a + b * sin_a;
	v[x1] = -a * sin_a + b * cos_a;
}



int main() {

	system("chcp 1251>nul");

	hullnum = 0;
	verbose = qfalse;

	planecount = &numbrushplanes;
	
	int i;
	entity_t* ent;
	face_t *f;
	mface_t* mf;
	mbrush_t* mb;
	surface_t* surf, *s;

	BeginBSPFile();

	ent = malloc(sizeof(entity_t));
	memset(ent, 0, sizeof(entity_t));

	vec3_t origin = { 100, 100, -10 };
	
	vec3_t sizes = { 100, 100, 100 };

	srand(time(NULL));

	for (int i = 0; i < 10; i++) {
		AddBoxToEntity(ent, origin, sizes, CONTENTS_SOLID, i);

		origin[0] += 100;
		origin[1] += 100;
		origin[2] += 100;

	}
	

	/*for (mb = ent->brushes; mb; mb = mb->next) {
		for (mf = mb->faces; mf; mf = mf->next) {
			Rotate(mf->plane.normal, Q_PI * (1) / 180.0, 0);
			Rotate(mf->plane.normal, Q_PI * (3) / 180.0, 1);
			Rotate(mf->plane.normal, Q_PI * (4) / 180.0, 2);
		}
		UpdateFaces(mb->faces);
	}*/



	nummodels = 0;
	hullnum = 0;

	brushset = Brush_LoadEntity(ent, hullnum);
	surf = CSGFaces(brushset);

	node_t* node = SolidBSP(surf, qfalse, qtrue);
	FreeBrushset(brushset);

	PortalizeWorld(node);

	WriteNodePlanes(node);
	WriteDrawNodes(node);
	WritePortals(node);
	FreeAllPortals(node);
	FreeNode_r(node);

	nummodels = 0;
	hullnum = 1;

	brushset = Brush_LoadEntity(ent, hullnum);

	surf = CSGFaces(brushset);

	node = SolidBSP(surf, qfalse, qtrue);

	WriteNodePlanes(node);
	WriteClipNodes(node);
	BumpModel(hullnum);

	printf("\n");


	char* name = "D:/Великая папка/Программы/BSPENG/ogl_begin/negro.bsp";

	for (i = 0; name[i]; i++)
		bspfilename[i] = name[i];

	bspfilename[i] = '\0';

	FinishBSPFile();

	return 0;

}
