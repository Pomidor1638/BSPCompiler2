#pragma once

typedef struct mface_s {
	struct mface_s*  next;
	plane_t			plane;
	texinfo_t	  texinfo;
} mface_t;

typedef struct mbrush_s {
	struct mbrush_s* next;
	mface_t*        faces;
	int          contents;
} mbrush_t;

typedef struct epair_s {
	struct epair_s* next;
	char* key;
	char* value;
} epair_t;

typedef struct {
	vec3_t		origin;
	mbrush_t* brushes;
	epair_t* epairs;
} entity_t;

void TextureAxisFromPlane(plane_t* pln, vec3_t xv, vec3_t yv);

int FindTexinfo(texinfo_t* t);