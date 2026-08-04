#pragma once
#include "../../../common/allocator/allocator.h"

namespace tungsten::memory
{
    extern allocator::LinearAllocator permanent_zone;
    extern allocator::LinearAllocator level_zone;
    extern allocator::LinearAllocator frame_zone;
    extern allocator::LinearAllocator scratch_zone;
    
    bool init_zones(size_t permanent_size, size_t level_size, size_t frame_size, size_t scratch_size);

    void quit_zones();
}
