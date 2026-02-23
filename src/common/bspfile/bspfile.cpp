//
// Created by UBER_USER on 22.12.2025.
//

#include "bspfile.h"


int BSPMap::parse(const std::vector<byte>& mem_block)
{
    if (mem_block.size() < sizeof(dheader_t))
        return -1;

    const dheader_t* header = reinterpret_cast<const dheader_t*>(mem_block.data());

    if (header->version != BSPVERSION)
        return -2;

    auto copyLump = [&](int lump_id, auto& vector, size_t struct_size)
        {
            const lump_t& lump = header->lumps[lump_id];
            if (lump.filelen == 0)
            {
                vector.clear();
                return;
            }

            size_t count = lump.filelen / struct_size;
            vector.resize(count);
            std::memcpy(vector.data(), mem_block.data() + lump.fileofs, lump.filelen);
        };

    copyLump(LUMP_VERTEXES, vertexes, sizeof(dvec3_t));
    copyLump(LUMP_PLANES, planes, sizeof(dplane_t));
    copyLump(LUMP_FACES, faces, sizeof(dface_t));
    copyLump(LUMP_NODES, nodes, sizeof(dnode_t));
    copyLump(LUMP_LEAFS, leafs, sizeof(dleaf_t));
    copyLump(LUMP_CLIPNODES, clipnodes, sizeof(dclipnode_t));
    copyLump(LUMP_MODELS, models, sizeof(dmodel_t));
    copyLump(LUMP_TEXINFO, texture_infos, sizeof(dtexinfo_t));
    copyLump(LUMP_PORTALS, portals, sizeof(dportal_t));

    copyLump(LUMP_VINDEXES, vertex_indexes, sizeof(dvec3_index));
    copyLump(LUMP_PINDEXES, portal_indexes, sizeof(dportal_index));

    auto copyRawLump = [&](int lump_id, std::vector<byte>& block)
        {
            const lump_t& lump = header->lumps[lump_id];
            if (lump.filelen > 0)
            {
                block.resize(lump.filelen);
                std::memcpy(block.data(), mem_block.data() + lump.fileofs, lump.filelen);
            }
            else {
                block.clear();
            }
        };

    copyRawLump(LUMP_TEXTURES, texture_block);
    copyRawLump(LUMP_LIGHTING, lighting_block);
    copyRawLump(LUMP_VISIBILITY, visibility_block);

    return 0;
}

