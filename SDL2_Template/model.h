#pragma once

#include <SDL_opengl.h>
#include <vector>
#include <string>

#include "bspfile.h"
#include "mathlib.h"

typedef struct model_s {

	vec3_t mins;
	vec3_t maxs;
	vec3_t origin;

	int	headnode[MAX_MAP_HULLS];

	int	visleafs;

	int	firstface;
	int numfaces;

} model_t;

enum PLANE_TYPE : uint8_t {

	PLANE_AXIS_ANY = 0,


	PLANE_AXIS_X,
	PLANE_AXIS_Y,
	PLANE_AXIS_Z,

	PLANE_TYPE_XY,
	PLANE_TYPE_XZ,
	PLANE_TYPE_YZ,

};


typedef struct plane_s {
	
	vec3_t normal;
	vec_t dist;

	PLANE_TYPE type;

	vec_t dot(vec3_t origin) const {
		return normal * origin - dist;
	}

} plane_t;

typedef struct face_s {

	int         planenum;
	bool            side;
	int		  texinfonum;

	uint32_t  firstpoint;
	uint16_t   numpoints;

} face_t;

typedef struct node_s{

	vec3_t mins;
	vec3_t maxs;

	int planenum;
	int children[2];

} node_t;

typedef struct leaf_s {

	CONTENTS_TYPE contents;

	vec3_t mins;
	vec3_t maxs;

	uint32_t firstface;
	uint16_t numfaces;

	uint32_t firstportal;
	uint16_t numportals;

} leaf_t;

typedef struct texinfo_s {

	vec3_t u;
	vec3_t v;

	vec_t uoffset;
	vec_t voffset;

	int flags;
	int texturenum;

} texinfo_t;

enum TextureSize {
	TEX_2x2 = 2,
	TEX_4x4 = 4,
	TEX_16x16 = 16,
	TEX_32x32 = 32,
	TEX_64x64 = 64,     // Рекомендуемый по умолчанию
	TEX_128x128 = 128,
	TEX_256x256 = 256,
	TEX_512x512 = 512,
	TEX_1024x1024 = 1024,

	TEX_ANY
};


typedef struct texture_s {

	std::string name;

	uint16_t width;
	uint16_t height;

	GLuint gl_texturenum;

	~texture_s() {
		if (gl_texturenum != 0) {
			glDeleteTextures(1, &gl_texturenum);
		}
	}

} texture_t;



class Map {
public:
	std::string                    name;

	std::vector<texture_t>     textures;
	std::vector<texinfo_t>     texinfos;

	std::vector<vec3_t>		   vertexes;
	std::vector<uint32_t>      vindexes;

	std::vector<face_t>			  faces;

	std::vector<dportal_t>      portals; // Пока пусть будет dportal_t
	std::vector<uint32_t>      pindexes;

	std::vector<plane_t>         planes;
	std::vector<dclipnode_t>  clipnodes;
	std::vector<node_t>           nodes;
	std::vector<leaf_t>		      leafs;

	std::vector<model_t>         models;


protected:
	bool load(BSPMap* bspmap);
	bool loadTextures(BSPMap* bspmap);
public:

	bool load(std::string bspfilename);
	std::string toString();

};

