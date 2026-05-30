#include "../../../common/utils/allocator/allocator.h"


namespace tungsten::memory 
{
    extern util::allocator::LinearAllocator permanent_zone;
    extern util::allocator::LinearAllocator level_zone;
    extern util::allocator::LinearAllocator frame_zone;
    extern util::allocator::LinearAllocator scratch_zone;
}