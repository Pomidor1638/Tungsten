//
// Created by UBER_USER on 22.12.2025.
//

#include "bspfile.h"

#ifdef TUNGSTEN_ENGINE

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

#include <stdexcept>
#include <format>
#include <string>



bool BSPMap::parse_textures(const std::vector<uint8_t>& mem_block)
{
    const dheader_t* header = reinterpret_cast<const dheader_t*>(mem_block.data());
    lump_t lump = header->lumps[LUMP_TEXTURES];

    textures.clear();

    if (!lump.filelen)
    {
        return true;
    }

    if (lump.fileofs + lump.filelen > mem_block.size())
    {
        return false;
    }


    auto texblock = reinterpret_cast<const uint8_t*>(mem_block.data() + lump.fileofs);
    auto texlump = reinterpret_cast<const dtexturelump_t*>(texblock);

    const size_t texheader_size = sizeof(dtexturelump_t) + texlump->numtex * sizeof(texlump->dataofs[0]);

    textures.resize(texlump->numtex);

    for (size_t i = 0; i < texlump->numtex; i++)
    {
        size_t texofs = texlump->dataofs[i];
        auto& texture = textures[i];

        if (texofs == -1 || (size_t)texofs < texheader_size || (size_t)texofs > lump.filelen)
        {
            texture.color_type = -1;
            texture.height = -1;
            texture.width = -1;
            texture.name = "";
            continue;
        }

        auto dtexture = reinterpret_cast<const dtexture_t*>(texblock + texofs);
        size_t data_size = (size_t)dtexture->width * dtexture->height * dtexture->colortype;

        if (texofs + sizeof(dtexture_t) + data_size > lump.filelen)
            return false;

        // base initialization
        texture.color_type = dtexture->colortype;
        texture.height = dtexture->height;
        texture.width = dtexture->width;

        // to avoid overflow
        texture.name.assign(dtexture->name, strnlen_s(dtexture->name, MAX_MAP_TEXTURE_NAME));


        texture.data.resize(data_size);
        memcpy(texture.data.data(), dtexture->data, data_size);
    }

    return true;
}

bool BSPMap::parse_visibility(const std::vector<uint8_t>& mem_block)
{
    return true;
}
bool BSPMap::parse_lighting(const std::vector<uint8_t>& mem_block)
{
    return true;
}
bool BSPMap::parse_hearing(const std::vector<uint8_t>& mem_block)
{
    return true;
}

bool BSPMap::parse(const std::vector<uint8_t>& mem_block)
{
    if (mem_block.size() < sizeof(dheader_t))
        return false;

    const dheader_t* header = reinterpret_cast<const dheader_t*>(mem_block.data());

    if (header->version != BSPVERSION)
        return false;

    auto copyLump = [&](int lump_id, auto& vector, size_t struct_size) -> bool
        {
            vector.clear();
            const lump_t& lump = header->lumps[lump_id];
            if (lump.filelen == 0)
            {
                return true;
            }

            if (lump.fileofs + lump.filelen > mem_block.size())
                return false;

            size_t count = lump.filelen / struct_size;
            vector.resize(count);
            std::memcpy(vector.data(), mem_block.data() + lump.fileofs, lump.filelen);

            return true;
        };

    bool ok =
        copyLump(LUMP_VERTEXES, vertexes, sizeof(dvec3_t)) &&
        copyLump(LUMP_PLANES, planes, sizeof(dplane_t)) &&
        copyLump(LUMP_FACES, faces, sizeof(dface_t)) &&
        copyLump(LUMP_NODES, nodes, sizeof(dnode_t)) &&
        copyLump(LUMP_LEAFS, leafs, sizeof(dleaf_t)) &&
        copyLump(LUMP_CLIPNODES, clipnodes, sizeof(dclipnode_t)) &&
        copyLump(LUMP_MODELS, models, sizeof(dmodel_t)) &&
        copyLump(LUMP_TEXINFO, texture_infos, sizeof(dtexinfo_t)) &&
        copyLump(LUMP_PORTALS, portals, sizeof(dportal_t)) &&
        copyLump(LUMP_VINDEXES, vertex_indexes, sizeof(dvec3_index)) &&
        copyLump(LUMP_PINDEXES, portal_indexes, sizeof(dportal_index)) &&
        parse_textures(mem_block) &&
        parse_visibility(mem_block) &&
        parse_lighting(mem_block) &&
        parse_hearing(mem_block);

    if (!ok)
    {
        clear();
    }

    return ok; // Успех
}


void BSPMap::clear()
{
    vertexes.       clear();
    vertex_indexes. clear();
    planes.         clear();
    faces.          clear();
    portals.        clear();
    portal_indexes. clear();
    nodes.          clear();
    clipnodes.      clear();
    leafs.          clear();
    models.         clear();
    texture_infos.  clear();
    textures.       clear();
    lighting.       clear();
    visibility.     clear();
}

#endif