

#pragma once 

#include "../shared.h"

namespace tungsten::twad
{
    constexpr char      TMATERIAL_MAGIC[]    = "TUNGSTEN TWAD MATERIAL";
    constexpr size_t    TMATERIAL_MAGIC_SIZE = sizeof(TMATERIAL_MAGIC) - 1;
    constexpr uint64_t  TMATERIAL_VERSION    = 1;

    enum class tmaterial_lump_type : uint8_t
    {
        sequences = 0,
        frames,
        textures,
        normalsmaps,

        LUMPS_COUNT
    };

    constexpr size_t MATERIAL_LUMPS_COUNT = static_cast<size_t>(tmaterial_lump_type::LUMPS_COUNT);
    using twad_materials_header = twad_header<MATERIAL_LUMPS_COUNT, 0, TMATERIAL_MAGIC_SIZE>;

    constexpr size_t MAX_MATERIAL_NAME_SIZE = 32;
    struct twad_material
    {
        char        name[MAX_MATERIAL_NAME_SIZE];
        uint64_t    duration_us;
        uint32_t    frames_count;
    };

    enum class tmaterial_component
    {
        texture,
        normalmap,

        COMPONENTS_COUNT
    };

    constexpr size_t TMATERIAL_COMPONENTS_COUNT = static_cast<size_t>(tmaterial_component::COMPONENTS_COUNT);
    struct tmaterial_material_frame
    {
        uint64_t component_offsets[TMATERIAL_COMPONENTS_COUNT];
    };

    enum class twad_texture_format : uint8_t
    {
        none = 0,
        // indexed,

        rgb888,
        rgba8888,
        argb8888,
    };

    struct twad_texture_header
    {
        twad_texture_format format;
        uint16_t            width;
        uint16_t            height;
    };
};