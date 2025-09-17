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


typedef struct {
	
	vec3_t normal;
	vec_t dist;

	PLANE_TYPE type;

} plane_t;

typedef struct face_s {

	int         planenum;
	bool            side;
	int		  texinfonum;

	uint32_t  firstpoint;
	uint16_t   numpoints;

} face_t;
/*
typedef struct portal_t {

	int planenum;

	int leafs[2];

	uint32_t firstpoint;
	uint16_t numpoints;

} portal_t;
*/
typedef struct node_s{

	vec3_t mins;
	vec3_t maxs;

	int planenum;
	int children[2];

} node_t;
/*
typedef struct clipnode_s {

	int planenum;
	int children[2];

} clipnode_t;
*/
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

typedef struct texture_s {

	std::string name;

	uint16_t width;
	uint16_t height;

	GLuint gl_texturenum;

} texture_t;




struct Map {
	
	std::string                    name;

	std::vector<texture_t>     textures;
	std::vector<GLuint>        tindexes;
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

	bool load(BSPMap* bspmap);
	bool load(std::string bspfilename);
	std::string toString();

};

