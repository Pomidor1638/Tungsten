//
// Created by UBER_USER on 22.12.2025.
//

/*
	common .tbsp format in TUNGSTEN
*/

#pragma once

#include <cstdint>
#define TUNGSTEN_ENGINE

// is this need?
// typedef uint8_t byte
// I don't like using `byte`, it looks crude 

// need to replace by constexpr vars, but depricated tbsp (modified qbsp) shares this
#define PLANENUM_LEAF               -1
#define MAX_MAP_HULLS                4
#define MAX_MAP_MODELS             256
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
#define MAX_MAP_TEXTURE_NAME	   128
#define	MAX_MAP_MIPTEX		  0xF00000
#define	MAX_MAP_LIGHTING	  0x800000
#define	MAX_MAP_VISIBILITY	  0x800000
#define	MAX_MAP_HEARING	  	  0x800000

// need to fix

// it's not the actual version, it's like magic - only for integrity check

#define BSPVERSION            	   144



// Maybe it's property of .tmap format? need to specify.

// TODO:
// constexpr size_t MAX_MAP_SIZE = 


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

// this is a bad way to handle serialization. 
// it is better to use `binary_reader` from `util::binary`.

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
	dvec3_t 	 mins;
	dvec3_t 	 maxs;
	dvec3_t 	 origin;

	dnode_index	 headnode[MAX_MAP_HULLS];
	uint16_t     visleafs;

	dface_index	 firstface;
	uint16_t     numfaces;
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
	dplane_index 	planenum;
	uint8_t			side;

	dtexinfo_index 	texinfonum;

	dvec3_index 	firstpoint;
	uint16_t    	numpoints;
} dface_t;

typedef struct
{
	dplane_index planenum;
	dnode_index  leafs[2];

	dvec3_index	 firstpoint;
	uint16_t 	 numpoints;
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
	int8_t 		  contents;

	dvec3_t 	  mins;
	dvec3_t 	  maxs;

	dface_index   firstface;
	uint16_t      numfaces;

	dportal_index firstportal;
	uint16_t      numportals;
} dleaf_t;

typedef struct 
{
	float    vecs[2][4];
	uint32_t texnum;
	uint32_t flags;
} dtexinfo_t;

// Texture block

typedef struct
{
	uint32_t numtex;
	uint32_t dataofs[];		// [numtex]
} dtexturelump_t;

typedef struct dtexture_s
{
	char	 name[MAX_MAP_TEXTURE_NAME];
	uint32_t width, height;
	uint8_t	 colortype;
	uint8_t  data[];
} dtexture_t;

// lighting block
typedef struct dlightlump_s
{
	uint32_t numlight_masks;
	uint32_t dataofs[];
} dlightlump_t;

#pragma pack(pop) 


#ifdef TUNGSTEN_ENGINE

// depricated, need to replace

// never use heap allocation in tungsten
// use only linear allocators from tungsten::memory

#include <string>
#include <vector>
#include <bitset>


struct BSPTexture
{
	std::string name;
	int 		width, height;
	uint8_t 	color_type;

	std::vector<uint8_t> data{};
};

struct BSPLightMask
{
	int 	width, height;
	uint8_t light_type;

	std::vector<uint8_t> data;
};

struct BSPVisibility // PVS
{
	int size;
	std::vector<bool> PVS;
};

// TODO:
// struct BSPHearing // PHS
// {
// 	int size;
// 	std::bitset<MAX_MAP_HEARING_SIZE> PHS;
// };


// is this struct? Should it be as class ?
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

	std::vector<BSPTexture>          textures{};
	std::vector<BSPLightMask>        lighting{};
	std::vector<BSPVisibility>     visibility{};


	BSPMap() = default;
	virtual ~BSPMap() = default;

	bool parse(const std::vector<uint8_t>& mem_block);
	void clear();

private:
	bool parse_textures(const std::vector<uint8_t>& mem_block);
	bool parse_visibility(const std::vector<uint8_t>& mem_block);
	bool parse_lighting(const std::vector<uint8_t>& mem_block);
	bool parse_hearing(const std::vector<uint8_t>& mem_block);
};

#endif
