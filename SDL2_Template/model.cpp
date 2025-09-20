#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <set>
#include "engine.h"



static vec3_t makeVec3(dvertex_t dv) {
	return vec3_t{
		dv.v[0],
		dv.v[1],
		dv.v[2],
	};
}

static vec3_t makeVec3(float v[3]) {
	return vec3_t{
		v[0],
		v[1],
		v[2],
	};
}


static model_t makeModel(dmodel_t dm) {
	model_t m;

	m.visleafs = dm.visleafs;
	m.firstface = dm.firstface;
	m.numfaces = dm.numfaces;

	m.headnode[0] = dm.headnode[0];
	m.headnode[2] = dm.headnode[1];
	m.headnode[3] = dm.headnode[2];

	m.maxs[0] = dm.maxs[0];
	m.maxs[1] = dm.maxs[1];
	m.maxs[2] = dm.maxs[2];

	m.mins[0] = dm.mins[0];
	m.mins[1] = dm.mins[1];
	m.mins[2] = dm.mins[2];


	m.origin[0] = dm.origin[0];
	m.origin[1] = dm.origin[1];
	m.origin[2] = dm.origin[2];

	return m;
}

static texinfo_t makeTexinfo(dtexinfo_t dt) {
	texinfo_t t;

	t.flags = dt.flags;

	t.texturenum = dt.miptexnum; // неверно, но пусть пока останется

	
	t.u = { dt.vecs[0][0], dt.vecs[0][1], dt.vecs[0][2] };
	t.v = { dt.vecs[1][0], dt.vecs[1][1], dt.vecs[1][2] };

	t.uoffset = dt.vecs[0][3];
	t.voffset = dt.vecs[1][3];

	return t;
}

static face_t makeFace(dface_t df) {
	face_t f;

	f.planenum = df.planenum;
	f.side = df.side;
	f.texinfonum = df.texinfonum;
	f.firstpoint = df.firstpoint;
	f.numpoints = df.numpoints;

	return f;
}

static plane_t makePlane(dplane_t dp) {
	plane_t p;

	dvertex_t dv;
	dv.v[0] = dp.normal[0]; dv.v[1] = dp.normal[1]; dv.v[2] = dp.normal[2];

	vec3_t normal = p.normal = makeVec3(dv);
	p.dist = dp.dist;


	int mask = (std::abs(normal[0]) > EPSILON_ZERO ? 1 : 0) |
			   (std::abs(normal[1]) > EPSILON_ZERO ? 2 : 0) |
			   (std::abs(normal[2]) > EPSILON_ZERO ? 4 : 0);

	switch (mask) {
	case 1: 
		p.type = (std::abs(normal[0] - 1.0) < EPSILON_ZERO) ? PLANE_AXIS_X : PLANE_AXIS_ANY;
		break;
	case 2: 
		p.type = (std::abs(normal[1] - 1.0) < EPSILON_ZERO) ? PLANE_AXIS_Y : PLANE_AXIS_ANY;
		break;
	case 4:
		p.type = (std::abs(normal[2] - 1.0) < EPSILON_ZERO) ? PLANE_AXIS_Z : PLANE_AXIS_ANY;
		break;
	case 3:
		p.type = PLANE_TYPE_XY;
		break;
	case 5:
		p.type = PLANE_TYPE_XZ;
		break;
	case 6:
		p.type = PLANE_TYPE_YZ;
		break;
	default:
		p.type = PLANE_AXIS_ANY;
	}
	
	return p;
}

static node_t makeNode(dnode_t dnode)
{
	node_t node;

	node.mins = makeVec3(dnode.mins);
	node.maxs = makeVec3(dnode.maxs);

	node.planenum = dnode.planenum;
	node.children[0] = dnode.children[0];
	node.children[1] = dnode.children[1];

	return node;
}

static leaf_t makeLeaf(dleaf_t dleaf)
{
	leaf_t leaf;
	
	leaf.contents = CONTENTS_TYPE(dleaf.contents);

	leaf.mins = makeVec3(dleaf.mins);
	leaf.maxs = makeVec3(dleaf.maxs);

	leaf.firstface = dleaf.firstface;
	leaf.numfaces  = dleaf.numfaces;

	leaf.firstportal = leaf.firstportal;
	leaf.numportals  = leaf.numportals;

	return leaf;
}

template <typename DST, typename SRC>
static bool loadThing(std::vector<DST>& dst, SRC* src, int num, int max, DST(*func)(SRC) = [](SRC elem) { return DST(elem); }) 
{
	if (!src || !num || num > max) {

		printf("");
		return false;
	}

	dst.resize(num);
	for (int i = 0; i < num; i++) 
	{
		dst[i] = func(src[i]);
	}
	return true;
}

bool loadBSPTextureData(miptex_t* mip, byte* textureData, texture_t& texture) {

	if (!mip || !textureData) {
		printf("Invalid texture data\n");
		return false;
	}

	int width = mip->width;
	int height = mip->height;

	if (width <= 0 || height <= 0 || width > 1024 || height > 1024) {
		printf("Invalid texture size: %dx%d\n", width, height);
		return false;
	}

	/*if (mip->offsets[0] == 0) {
		printf("Texture %s has invalid offset 0\n", mip->name);
		return false;
	}*/

	glBindTexture(GL_TEXTURE_2D, texture.gl_texturenum);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLint colortype = mip->colortype == 3 ? GL_RGB : GL_RGBA;

	glTexImage2D(
		GL_TEXTURE_2D, 0, colortype, width, height, 0,
		colortype, GL_UNSIGNED_BYTE, textureData
	);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("OpenGL error during texture loading: %d\n", err);
		glBindTexture(GL_TEXTURE_2D, 0);
		return false;
	}

	texture.name = mip->name;
	texture.width = width;
	texture.height = height;

	glBindTexture(GL_TEXTURE_2D, 0);

	printf("BSP Texture %s: ID %u, Size %dx%d\n",
		texture.name.c_str(), texture.gl_texturenum, texture.width, texture.height);

	return true;
}

bool Map::loadTextures(BSPMap* bspmap) {

	if (!bspmap)
		return false;

	std::vector<GLuint> texturesindexes(MAX_MAP_TEXTURES);

	glGenTextures(texturesindexes.size(), texturesindexes.data());

	dmiptexlump_t* lump = (dmiptexlump_t*)bspmap->dtexdata;

	int nummiptex = lump->nummiptex;

	if (nummiptex <= 0 || nummiptex > MAX_MAP_TEXTURES)
	{
		printf("Can't load textures: %i nummiptex\n", nummiptex);
		return false;
	}

	textures.resize(nummiptex);

	for (int i = 0; i < nummiptex; i++) {

		int offset = lump->dataofs[i];
		if (offset == -1) {
			printf("Warning: %i texture doesn't exist\n", i);
		}

		miptex_t* mip = (miptex_t*)&bspmap->dtexdata[offset];
		byte* textureData = (byte*)(mip) + sizeof(miptex_t);

		printf("%i nigger\n", (int)sizeof(miptex_t));

		texture_t& t = textures[i];
		t.gl_texturenum = texturesindexes[i];

		if (loadBSPTextureData(mip, textureData, t))
		{
			printf("Loaded Texture \'%s\', width: %i, height: %i\n", t.name.c_str(), t.width, t.height);
		}
		else
		{
			printf("Can't load %i texture %s\n", i, t.name.c_str());
		}

	}
	return true;
}

bool Map::load(BSPMap* bspmap) 
{

	if (!bspmap) 
	{
		return false;
	}

	name = bspmap->bspfilename;

	bool load =
		loadThing(vertexes , bspmap->dvertexes , bspmap->numvertexes , MAX_MAP_VERTEXES     , makeVec3   ) and
		loadThing(vindexes , bspmap->dvindexes , bspmap->numvindexes , MAX_MAP_VERTEXINDEXES             ) and
		loadThing(faces    , bspmap->dfaces    , bspmap->numfaces    , MAX_MAP_FACES        , makeFace   ) and
		loadThing(portals  , bspmap->dportals  , bspmap->numportals  , MAX_MAP_PORTALS                   ) and
		loadThing(pindexes , bspmap->dpindexes , bspmap->numpindexes , MAX_MAP_PORTALINDEXES             ) and
		loadThing(planes   , bspmap->dplanes   , bspmap->numplanes   , MAX_MAP_PLANES       , makePlane  ) and
		loadThing(clipnodes, bspmap->dclipnodes, bspmap->numclipnodes, MAX_MAP_CLIPNODES                 ) and
		loadThing(nodes    , bspmap->dnodes    , bspmap->numnodes    , MAX_MAP_NODES        , makeNode   ) and
		loadThing(leafs    , bspmap->dleafs    , bspmap->numleafs    , MAX_MAP_LEAFS        , makeLeaf   ) and
		loadThing(models   , bspmap->dmodels   , bspmap->nummodels   , MAX_MAP_MODELS       , makeModel  ) and
		loadThing(texinfos , bspmap->dtexinfo  , bspmap->numtexinfo  , MAX_MAP_TEXINFO      , makeTexinfo) and
		loadTextures(bspmap);

	return load;
}

bool Map::load(std::string bspfilename) {

	BSPMap bsp;
	bsp.loadMap(bspfilename);

	return load(&bsp);

}

std::string Map::toString() {

	char str[1024]{};
	memset(str, 0, sizeof(str));
	std::string s{};

	struct {
		const char* name;
		int count;
		int structSize;
	} l[] = {
		{ "numplanes     ", planes.size()     , sizeof(planes[0])},
		{ "numvertexes   ", vertexes.size()   , sizeof(vertexes[0])},
		{ "numvertextable", vindexes.size()   , sizeof(vindexes[0])},
		{ "numportals    ", portals.size()    , sizeof(portals[0])},
		{ "numportaltable", pindexes.size()   , sizeof(pindexes[0])},
		{ "numfaces      ", faces.size()      , sizeof(faces[0])},
		{ "numnodes	     ", nodes.size()	  , sizeof(nodes[0])},
		{ "numleafs	     ", leafs.size()	  , sizeof(leafs[0])},
		{ "numclipnodes  ", clipnodes.size()  , sizeof(clipnodes[0])},
		//{ "numdtexinfo   ", dtexinfo   , sizeof(dtexinfo_t)     },
		//{ "lightdatasize ", lightdatasize , sizeof(byte)/* What? */},
		//{ "entdatasize   ", entdatasize   , sizeof(char)/*   ?   */},
		{ "nummodels     ", models.size()     , sizeof(models[0])}
	};

	sprintf_s(str, "name: %s\n", name.c_str());
	s += str;
	for (const auto& x : l) {
		sprintf_s(str, "%6i (bytes: %8i) %s \n", x.count, x.structSize * x.count, x.name);
		s += str;
	}

	return s;
}