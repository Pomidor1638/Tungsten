#pragma once

#include "twad_local.h"

namespace tungsten::twad
{

    constexpr char      TWAD_MAGIC[]     = "TUNGSTEN TWAD TOP";
    constexpr size_t    TWAD_MAGIC_SIZE  = sizeof(TWAD_MAGIC) - 1;
    constexpr uint64_t  TWAD_VERSION     = 1;

    // ---- //
    // TWAD //
    // ---- //

    enum class twad_top_lump_type : uint8_t
    {
        material,
        sound,
        model,
        prog,
        
        LUMPS_COUNT
    };

    constexpr size_t TWAD_LUMPS_COUNT = static_cast<size_t>(twad_top_lump_type::LUMPS_COUNT);
    constexpr size_t TWAD_MAX_NAME_SIZE = 32;

    using twad_top_header = twad_header<TWAD_LUMPS_COUNT, TWAD_MAX_NAME_SIZE, TWAD_MAGIC_SIZE>;

}
