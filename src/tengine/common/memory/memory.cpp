
#include "memory.h"


namespace tungsten::memory 
{
    util::allocator::LinearAllocator permanent_zone;
    util::allocator::LinearAllocator level_zone;
    util::allocator::LinearAllocator frame_zone;
    util::allocator::LinearAllocator scratch_zone;
}
