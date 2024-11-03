#include "bspfile.h"



int			numleafs;
dleaf_t		dleafs[MAX_MAP_LEAFS];

int			numplanes;
dplane_t	dplanes[MAX_MAP_PLANES];

int			numnodes;
dnode_t		dnodes[MAX_MAP_NODES];

int			numtexinfo;
texinfo_t	texinfo[MAX_MAP_TEXINFO];

int			numfaces;
dface_t		dfaces[MAX_MAP_FACES];

int			numclipnodes;
dclipnode_t	dclipnodes[MAX_MAP_CLIPNODES];

typedef struct {
	int		fileofs, filelen;
} lump_t;



void PrintBSPFileSizes(void)
{
	printf("%5i planes       %6i\n"
		, numplanes, (int)(numplanes * sizeof(dplane_t)));
	printf("%5i nodes        %6i\n"
		, numnodes, (int)(numnodes * sizeof(dnode_t)));
	printf("%5i texinfo      %6i\n"
		, numtexinfo, (int)(numtexinfo * sizeof(texinfo_t)));
	printf("%5i faces        %6i\n"
		, numfaces, (int)(numfaces * sizeof(dface_t)));
	printf("%5i clipnodes    %6i\n"
		, numclipnodes, (int)(numclipnodes * sizeof(dclipnode_t)));
	printf("%5i leafs        %6i\n"
		, numleafs, (int)(numleafs * sizeof(dleaf_t)));
	printf("%5i marksurfaces %6i\n"
		, numfaces, (int)(numfaces * sizeof(dfaces[0])));
	/*if (!texdatasize)
		printf("    0 textures          0\n");
	else*/
	//	printf("%5i textures     %6i\n", ((dmiptexlump_t*)dtexdata)->nummiptex, texdatasize);
	//printf("      lightdata    %6i\n", lightdatasize);
	//printf("      visdata      %6i\n", visdatasize);
	//printf("      entdata      %6i\n", entdatasize);
}