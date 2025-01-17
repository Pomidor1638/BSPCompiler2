#include "bspfile.h"


int			   nummodels;
dmodel_t	   dmodels[MAX_MAP_MODELS];
			   
int			   lightdatasize;
byte		   dlightdata[MAX_MAP_LIGHTING];
			   
int			   entdatasize;
char		   dentdata[MAX_MAP_ENTSTRING];
			   
int			   numleafs;
dleaf_t		   dleafs[MAX_MAP_LEAFS];
			   
int			   numplanes;
dplane_t       dplanes[MAX_MAP_PLANES];
			   
int			   numnodes;
dnode_t		   dnodes[MAX_MAP_NODES];
			   
int			   numdtexinfo;
dtexinfo_t	   dtexinfo[MAX_MAP_TEXINFO];
			   
int			   numvert;
dvertex_t	   dvertexes[MAX_MAP_VERTS];

int			   numvertextable;
unsigned int   dvertextable[MAX_MAP_VERTEXTABLE];

int			   numfaces;
dface_t		   dfaces[MAX_MAP_FACES];
			   
int			   numportals;
dportal_t	   dportals[MAX_MAP_PORTALS];
			   
int			   numportaltable;
unsigned short  dportaltable[MAX_MAP_PORTALTABLE];
			   
int			   numclipnodes;
dclipnode_t	   dclipnodes[MAX_MAP_CLIPNODES];

FILE* bspfile;
char bspfilename[1024];

dheader_t header;

void PrintBSPFileSizes(void) {
	printf("%6i planes         %7i\n", numplanes, (int)(numplanes * sizeof(dplane_t)));
	printf("%6i nodes          %7i\n", numnodes, (int)(numnodes * sizeof(dnode_t)));
	printf("%6i texinfo        %7i\n", numdtexinfo, (int)(numdtexinfo * sizeof(dtexinfo_t)));
	printf("%6i vertexes       %7i\n", numvert, (int)(numvert * sizeof(dvertex_t)));
	printf("%6i vertindexes    %7i\n", numvertextable, (int)(numvertextable * sizeof(dvertextable[0])));
	printf("%6i faces          %7i\n", numfaces, (int)(numfaces * sizeof(dface_t)));
	printf("%6i portals        %7i\n", numportals, (int)(numportals * sizeof(dportal_t)));
	printf("%6i portalindexes  %7i\n", numportaltable, (int)(numportaltable * sizeof(dportaltable[0])));
	printf("%6i clipnodes      %7i\n", numclipnodes, (int)(numclipnodes * sizeof(dclipnode_t)));
	printf("%6i leafs          %7i\n", numleafs, (int)(numleafs * sizeof(dleaf_t)));
	printf("%6i models         %7i\n", nummodels, (int)(nummodels * sizeof(dmodel_t)));
}



FILE* SafeOpenWrite(char* filename) {
	FILE* f;

	fopen_s(&f, filename, "wb");

	if (!f)
		Error("Can`t open %s\n", filename);

	return f;
}

FILE* SafeOpenRead(char* filename) {
	FILE* f;

	fopen_s(&f, filename, "wr");

	if (!f)
		Error("Can`t open %s\n", filename);

	return f;
}

void SafeRead(FILE* f, void* buffer, size_t count) {
	if (fread(buffer, 1, count, f) != (size_t)count)
		Error("File read failure");
}

void SafeWrite(FILE* f, void* buffer, size_t count) {
	if (fwrite(buffer, 1, count, f) != (size_t)count)
		Error("File read failure");
}

void AddLump(int lumpnum, void* buffer, size_t size) {

	lump_t* lump = &header.lumps[lumpnum];

	lump->fileofs = ftell(bspfile);
	lump->filelen = size;

	SafeWrite(bspfile, buffer, size);
}

void WriteBSPFile() {

	memset(&header, 0, sizeof(dheader_t));

	header.version = BSPVERSION;

	bspfile = SafeOpenWrite(bspfilename);
	SafeWrite(bspfile, &header, sizeof(dheader_t));	// overwritten later

	AddLump(LUMP_PLANES, dplanes, numplanes * sizeof(dplane_t));
	AddLump(LUMP_VERTEXES, dvertexes, numvert * sizeof(dvertex_t));
	AddLump(LUMP_VTABLE, dvertextable, numvertextable * sizeof(dvertextable[0]));
	AddLump(LUMP_PORTALS, dportals, numportals * sizeof(dportal_t));
	AddLump(LUMP_PORTALTABLE, dportaltable, numportaltable * sizeof(dportaltable[0]));
	AddLump(LUMP_FACES, dfaces, numfaces * sizeof(dface_t));
	AddLump(LUMP_NODES, dnodes, numnodes * sizeof(dnode_t));
	AddLump(LUMP_LEAFS, dleafs, numleafs * sizeof(dleaf_t));
	AddLump(LUMP_CLIPNODES, dclipnodes, numclipnodes * sizeof(dclipnode_t));
	AddLump(LUMP_TEXINFO, dtexinfo, numdtexinfo * sizeof(dtexinfo_t));
	AddLump(LUMP_LIGHTING, dlightdata, lightdatasize);
	AddLump(LUMP_ENTITIES, dentdata, entdatasize);
	AddLump(LUMP_MODELS, dmodels, nummodels * sizeof(dmodel_t));

	fseek(bspfile, 0, SEEK_SET);
	SafeWrite(bspfile, &header, sizeof(dheader_t));
	fclose(bspfile);

}

void BeginBSPFile() {
	numleafs = 1;
	numdtexinfo = 1;
	dleafs[0].contents = CONTENTS_SOLID;
}

void FinishBSPFile(void) {

	printf("*============* FinishBSPFile *===========*\n");
	printf("WriteBSPFile: %s\n", bspfilename);

	PrintBSPFileSizes();
	WriteBSPFile(bspfilename);

}
