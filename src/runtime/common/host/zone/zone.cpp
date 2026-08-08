
#include "zone.h"

namespace tungsten::zone
{

    namespace
    {
        bool memory_initialized = false;
    }

    allocator::LinearAllocator permanent_zone;
    allocator::LinearAllocator level_zone;
    allocator::LinearAllocator frame_zone;
    allocator::LinearAllocator scratch_zone;

    bool init(void* memory_ptr, size_t memory_size, size_t permanent_size, size_t level_size, size_t frame_size, size_t scratch_size)
    {
        if (!memory_ptr)
        {
            return false;
        }

        size_t total_requested = permanent_size + level_size + frame_size + scratch_size;
        
        if (total_requested > memory_size)
        {
            return false;
        }


        uint8_t* current_ptr = static_cast<uint8_t*>(memory_ptr);

        permanent_zone.setup_buffer(current_ptr, permanent_size);
        current_ptr += permanent_size;

        level_zone.setup_buffer(current_ptr, level_size);
        current_ptr += level_size;

        frame_zone.setup_buffer(current_ptr, frame_size);
        current_ptr += frame_size;

        scratch_zone.setup_buffer(current_ptr, scratch_size);


        return true;
    }

    void quit()
    {
        if (!memory_initialized)
            return;

        permanent_zone.reset();
        level_zone.reset();
        frame_zone.reset();
        scratch_zone.reset();
    }
}
