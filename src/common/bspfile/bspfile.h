//
// Created by UBER_USER on 22.12.2025.
//

#ifndef MEGAGAME_BSPFILE_H
#define MEGAGAME_BSPFILE_H

#pragma once

#include <cstdint>
#include <vector>
#include <array>

typedef unsigned char byte;

#define PLANENUM_LEAF               -1
#define MAX_MAP_HULLS                4
#define MAX_MAP_MODELS             256
#define MAX_MAP_BRUSHES           4096
#define MAX_MAP_ENTITIES          1024
#define MAX_MAP_ENTSTRING        65536

#define MAX_MAP_PLANES            8192
#define MAX_MAP_NODES            32767
#define MAX_MAP_CLIPNODES        32767
#define MAX_MAP_LEAFS            32767    
#define MAX_MAP_VERTEXES         65535
#define MAX_MAP_VERTEXINDEXES   256000
#define MAX_MAP_FACES            65535    
#define MAX_MAP_PORTALS          65535
#define MAX_MAP_PORTALINDEXES   256000
#define MAX_MAP_TEXINFO           4096
#define MAX_TEXTURE_NAME	       128
#define MIP_MAP_LEVELS               4
#define	MAX_MAP_MIPTEX		  0xF00000
#define	MAX_MAP_LIGHTING	  0x100000
#define	MAX_MAP_VISIBILITY	  0x100000

#define BSPVERSION            144

enum LUMP_THINGS
{

	LUMP_PLANES = 0,
	LUMP_VERTEXES,
	LUMP_VINDEXES,
	LUMP_PORTALS,
	LUMP_PINDEXES,
	LUMP_FACES,
	LUMP_NODES,
	LUMP_LEAFS,
	LUMP_CLIPNODES,
	LUMP_TEXINFO,
	LUMP_TEXTURES,
	LUMP_LIGHTING,
	LUMP_VISIBILITY,
	LUMP_ENTITIES,
	LUMP_MODELS,

	HEADER_LUMPS
};

#pragma pack(push, 1)


typedef int32_t  dplane_index;   
typedef int32_t  dnode_index;    
typedef int32_t  dclipnode_index;
typedef uint32_t dface_index;    
typedef uint32_t dportal_index;  
typedef int32_t  dtexinfo_index;
typedef uint32_t dvec3_index;    


typedef float dvec_t;

typedef struct
{
	dvec_t v[3];
} dvec3_t;

typedef struct
{
	dvec3_t mins;
	dvec3_t maxs;
	dvec3_t origin;

	dnode_index	 headnode[MAX_MAP_HULLS];
	uint16_t     visleafs;

	dface_index	firstface;
	uint16_t    numfaces;

} dmodel_t;

typedef struct
{
	uint32_t fileofs;
	uint32_t filelen;
} lump_t;

typedef struct
{
	int32_t version;
	lump_t	lumps[HEADER_LUMPS];
} dheader_t;

typedef enum CONTENTS_TYPE
{
	CONTENTS_EMPTY = -1,
	CONTENTS_SOLID = -2,
	CONTENTS_WATER = -3,
	CONTENTS_SLIME = -4,
	CONTENTS_LAVA  = -5,
	CONTENTS_SKY   = -6,
} CONTENTS_TYPE;

typedef struct
{
	dvec3_t normal;
	dvec_t  dist;
} dplane_t;


typedef struct
{
	dplane_index planenum;
	byte side;

	dtexinfo_index texinfonum;

	dvec3_index firstpoint;
	uint16_t    numpoints;

} dface_t;

typedef struct
{
	dplane_index planenum;
	dnode_index  leafs[2];

	dvec3_index firstpoint;
	uint16_t numpoints;

} dportal_t;

typedef struct
{
	dvec3_t mins;
	dvec3_t maxs;

	dplane_index planenum;
	dnode_index  children[2];
} dnode_t;

typedef struct
{
	dplane_index    planenum;
	dclipnode_index children[2];
} dclipnode_t;

typedef struct
{
	int8_t contents;

	dvec3_t mins;
	dvec3_t maxs;

	dface_index firstface;
	uint16_t    numfaces;

	dportal_index firstportal;
	uint16_t      numportals;

} dleaf_t;

typedef struct {
	float vecs[2][4];
	int texnum;
	uint32_t flags;
} dtexinfo_t;

// Texture block

typedef struct
{
	int			numtex;
	int			dataofs[];		// [numtex]
} dtexturelump_t;

typedef struct dtexture_s
{
	char		name[MAX_TEXTURE_NAME];
	unsigned	width, height;
	byte		colortype;
} dtexture_t;

#pragma pack(pop) 


struct BSPMap
{
	std::vector<dvec3_t>             vertexes{};
	std::vector<dvec3_index>   vertex_indexes{};
	std::vector<dplane_t>              planes{};
	std::vector<dface_t>                faces{};
	std::vector<dportal_t>            portals{};
	std::vector<dportal_index> portal_indexes{};
	std::vector<dnode_t>                nodes{};
	std::vector<dclipnode_t>        clipnodes{};
	std::vector<dleaf_t>                leafs{};
	std::vector<dmodel_t>              models{};
	std::vector<dtexinfo_t>     texture_infos{};

	std::vector<byte>    texture_block{};
	std::vector<byte>   lighting_block{};
	std::vector<byte> visibility_block{};

	BSPMap() = default;
	virtual ~BSPMap() = default;


	int parse(const std::vector<byte>& mem_block);
};


#endif //MEGAGAME_BSPFILE_H