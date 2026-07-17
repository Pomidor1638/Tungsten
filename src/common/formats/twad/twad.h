#pragma once
#include <cstdint>

namespace tungsten::twad
{
#pragma pack(push, 1)

    constexpr char      MAGIC[]     = "TUNGSTEN TWAD";
    constexpr size_t    MAGIC_SIZE  = sizeof(MAGIC) - 1;
    constexpr uint64_t  VERSION     = 1;

    enum twad_lump_flags : uint64_t
    {
        TWAD_LUMP_FLAG_NONE         =      0,
        TWAD_LUMP_FLAG_COMPRESSED   = 1 << 0,
        TWAD_LUMP_FLAG_ENCRYPTED    = 1 << 1,
    };

    struct twad_lump
    {
        uint64_t flags;
        uint64_t checksum;
        
        uint64_t offset;
        uint64_t size;
    };

    enum class twad_top_lump_type : uint8_t
    {
        material,
        sound,
        model,
        prog,
        
        lumps_count
    };

    constexpr size_t LUMPS_COUNT = static_cast<size_t>(twad_top_lump_type::lumps_count);
    constexpr size_t TWAD_MAX_NAME_SIZE = 32;
    struct twad_header
    {
        uint8_t     magic[MAGIC_SIZE];    
        uint64_t    version;
        uint64_t    checksum;
        uint64_t    flags;
        uint64_t    total_size;
        char        name[TWAD_MAX_NAME_SIZE];
        twad_lump   lumps[LUMPS_COUNT];
    };

    // ------------------ //
    // TWAD_LUMP_MATERIAL //
    // ------------------ //

    // material is material_frames
    // material_frame is texture, normalsmap and etc.

    enum class twad_material_lump_type : uint8_t
    {
        materials = 0,
        frames,
        textures,
        normalsmaps,

        lumps_count
    };

    constexpr size_t MATERIAL_LUMPS_COUNT = static_cast<size_t>(twad_material_lump_type::lumps_count);
    struct twad_material_header
    {
        uint64_t    total_size;
        twad_lump   lumps[MATERIAL_LUMPS_COUNT];
    };

    constexpr size_t MAX_MATERIAL_NAME_SIZE = 32;
    struct twad_material
    {
        char        name[MAX_MATERIAL_NAME_SIZE];
        uint64_t    duration_us;
        uint32_t    frames_count;
        uint64_t    indexes[];    // [frames_count]
    };

    struct twad_material_frame
    {
        uint64_t    texture_offset;
        uint64_t normalsmap_offset;
    };

    enum class twad_texture_format : uint8_t
    {
        none = 0,
        // indexed,

        rgb888,
        rgba8888,
        argb8888,
    };

    struct twad_texture
    {
        twad_texture_format format;
        uint16_t            width;
        uint16_t            height;
        uint8_t             data[]; // [width*height*bpf], bfp - bytes per format
    };

#pragma pack(pop)
}
