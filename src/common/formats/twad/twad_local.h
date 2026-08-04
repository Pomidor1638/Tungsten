
#pragma once

#include <cstdint>
#include "../../utils/binary/binary.h"

namespace tungsten::twad
{
    enum twad_lump_flags : uint64_t
    {
        TWAD_LUMP_FLAG_NONE = 0,
        TWAD_LUMP_FLAG_COMPRESSED = 1 << 0,
        TWAD_LUMP_FLAG_ENCRYPTED = 1 << 1,
    };

    struct twad_lump
    {
        uint64_t flags;
        uint64_t checksum;

        uint64_t size;
        uint64_t offset;
    };


    template <size_t LUMPS_COUNT, size_t MAX_NAME_SIZE, size_t MAGIC_SIZE>
    struct twad_header
    {
        uint8_t     magic[MAGIC_SIZE];
        uint64_t    version;
        uint64_t    checksum;
        uint64_t    flags; // reserved
        uint64_t    total_size;
        char        name[MAX_NAME_SIZE];
        twad_lump   lumps[LUMPS_COUNT];
    };

    using twad_reader = util::binary::binary_reader<util::binary::bin_endian_type::big>;
    using twad_writer = util::binary::binary_writer<util::binary::bin_endian_type::big>;
}