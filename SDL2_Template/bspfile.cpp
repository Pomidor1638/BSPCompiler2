#include "bspfile.h"
#include <fstream>
#include <iostream>


void BSPMap::printMap() const {
	struct {
		const char* name;
		int count;
		int structSize;
	} l[] = {
		{ "numplanes     ", numplanes    , sizeof(dplane_t)       },
		{ "numvertexes   ", numvertexes  , sizeof(dvertex_t)      },
		{ "numvertextable", numvindexes  , sizeof(unsigned int)   },
		{ "numportals    ", numportals   , sizeof(dportal_t)      },
		{ "numportaltable", numpindexes  , sizeof(unsigned short) },
		{ "numfaces      ", numfaces     , sizeof(dface_t)        },
		{ "numnodes	     ", numnodes	 , sizeof(dnode_t)        },
		{ "numleafs	     ", numleafs	 , sizeof(dleaf_t)        },
		{ "numclipnodes  ", numclipnodes , sizeof(dclipnode_t)    },
		{ "numdtexinfo   ", numtexinfo   , sizeof(dtexinfo_t)     },
		{ "lightdatasize ", lightdatasize, sizeof(byte)/* What? */},
		{ "texdatasize   ", texdatasize  , sizeof(byte)/*   ?   */},
		{ "entdatasize   ", entdatasize  , sizeof(char)/*   ?   */},
		{ "nummodels     ", nummodels    , sizeof(dmodel_t)       }
	};

	printf("name: %s\n", bspfilename);

	for (const auto& x : l) {
		printf("%6i (bytes: %8i) %s \n", x.count, x.structSize* x.count, x.name);
	}

}

bool BSPMap::loadMap(std::string path) {

	std::ifstream file(path, std::ifstream::binary);

	if (!file.is_open()) {
		//printf("can't open\n");
		return false;
	}

	dheader_t header{};

	struct {
		LUMP_THINGS lump;
		int* count;
		void** strctptr;
		int structSize;
	} lumpsForLoad[] = {
		{ LUMP_PLANES   , &numplanes    , (void**)(&dplanes   ), sizeof(dplane_t)       },
		{ LUMP_VERTEXES , &numvertexes  , (void**)(&dvertexes ), sizeof(dvertex_t)      },
		{ LUMP_VINDEXES , &numvindexes  , (void**)(&dvindexes ), sizeof(unsigned int)   },
		{ LUMP_PORTALS  , &numportals   , (void**)(&dportals  ), sizeof(dportal_t)      },
		{ LUMP_PINDEXES , &numpindexes  , (void**)(&dpindexes ), sizeof(unsigned short) },
		{ LUMP_FACES    , &numfaces     , (void**)(&dfaces    ), sizeof(dface_t)        },
		{ LUMP_NODES    , &numnodes	    , (void**)(&dnodes    ), sizeof(dnode_t)        },
		{ LUMP_LEAFS    , &numleafs	    , (void**)(&dleafs    ), sizeof(dleaf_t)        },
		{ LUMP_CLIPNODES, &numclipnodes , (void**)(&dclipnodes), sizeof(dclipnode_t)    },
		{ LUMP_TEXINFO  , &numtexinfo   , (void**)(&dtexinfo  ), sizeof(dtexinfo_t)     },
		{ LUMP_LIGHTING , &lightdatasize, (void**)(&dlightdata), sizeof(byte)/* What? */},
		{ LUMP_TEXTURES , &texdatasize  , (void**)(&dtexdata  ), sizeof(byte)/*   ?   */},
		{ LUMP_ENTITIES , &entdatasize  , (void**)(&dentdata  ), sizeof(char)/*   ?   */},
		{ LUMP_MODELS   , &nummodels    , (void**)(&dmodels   ), sizeof(dmodel_t)       },
	};

	if (!file.read((char*)&header, sizeof(dheader_t))) {
		file.close();
		return false;
	}

	if (header.version != BSPVERSION) {
		file.close();
		//printf("bad version\n");
		return false;
	}

	for (auto& l : lumpsForLoad) {

		const lump_t& lump = header.lumps[l.lump];
		const int& len = lump.filelen;
		const int& ofs = lump.fileofs;

		file.seekg(ofs, std::ios::beg);

		if (*l.strctptr) {
			delete[] *l.strctptr;
			*l.strctptr = nullptr;
		}

		if (len <= 0) {
			*l.count = 0;
			*l.strctptr = nullptr;
			continue;
		}

		*l.count = len / l.structSize;
		*l.strctptr = new byte[len];
		if (!file.read(reinterpret_cast<char*>(*l.strctptr), len)) {
			file.close();

			for (auto& alloc : lumpsForLoad) {
				if (*alloc.strctptr) {
					delete[] * alloc.strctptr;
					*alloc.strctptr = nullptr;
					*alloc.count = 0;
				}
			}
			return false;
		}
	}

	bspfilename = path;

	file.close();
	return true;
}

BSPMap::~BSPMap() {
	void** pointers[] = {
		(void**)(&dplanes),
		(void**)(&dvertexes),
		(void**)(&dvindexes),
		(void**)(&dportals),
		(void**)(&dpindexes),
		(void**)(&dfaces),
		(void**)(&dnodes),
		(void**)(&dleafs),
		(void**)(&dclipnodes),
		(void**)(&dtexinfo),
		(void**)(&dtexdata),
		(void**)(&dlightdata),
		(void**)(&dentdata),
		(void**)(&dmodels),
		(void**)(&dtexturedata),
	};

	for (auto& p : pointers) {
		if (*p) {
			delete[] *p;
		}
		*p = nullptr;
	}
}