#include "qbsp.h"

vec3_t	baseaxis[18] = {
	{ 0, 0, 1}, { 1 ,0, 0}, { 0,-1, 0},			// floor
	{ 0, 0,-1}, { 1, 0, 0}, { 0,-1, 0},		// ceiling
	{ 1, 0, 0}, { 0, 1, 0}, { 0, 0,-1},			// west wall
	{-1, 0, 0}, { 0, 1, 0}, { 0, 0,-1},		// east wall
	{ 0, 1, 0}, { 1, 0, 0}, { 0, 0,-1},			// south wall
	{ 0,-1, 0}, { 1, 0, 0}, { 0, 0,-1}			// north wall
};

void TextureAxisFromPlane(plane_t* pln, vec3_t xv, vec3_t yv) {
	int		bestaxis;
	float	dot, best;
	int		i;

	best = 0;
	bestaxis = 0;

	for (i = 0; i < 6; i++) {
		dot = DotProduct(pln->normal, baseaxis[i * 3]);
		if (dot > best) {
			best = dot;
			bestaxis = i;
		}
	}

	VectorCopy(baseaxis[bestaxis * 3 + 1], xv);
	VectorCopy(baseaxis[bestaxis * 3 + 2], yv);
}

int FindTexinfo(texinfo_t* t) {
	int i;
	texinfo_t* cmp;

	cmp = texinfo;

	for (i = 0; i < numtexinfo; i++) {
		if (memcmp(t, cmp, sizeof(texinfo_t)) == 0)
			return i;
		cmp++;
	}

	if (numtexinfo == MAX_MAP_TEXINFO)
		Error("numtexinfo == MAX_MAP_TEXINFO");

	texinfo[numtexinfo] = *t;
	numtexinfo++;

	return i;
}