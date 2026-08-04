
#include "memory.h"



namespace tungsten::memory
{
    util::allocator::LinearAllocator permanent_zone;
    util::allocator::LinearAllocator level_zone;
    util::allocator::LinearAllocator frame_zone;
    util::allocator::LinearAllocator scratch_zone;

    constexpr size_t TOTAL_ENGINE_MEMORY = 128 * 1024 * 1024ull;
    alignas(16) static uint8_t g_mega_buffer[TOTAL_ENGINE_MEMORY];

    bool init_zones(size_t permanent_size, size_t level_size, size_t frame_size, size_t scratch_size)
    {
        size_t total_requested = permanent_size + level_size + frame_size + scratch_size;
        if (total_requested > TOTAL_ENGINE_MEMORY)
        {
            return false;
        }

        uint8_t* current_ptr = g_mega_buffer;

        permanent_zone.setup_buffer(current_ptr, permanent_size);
        current_ptr += permanent_size;

        level_zone.setup_buffer(current_ptr, level_size);
        current_ptr += level_size;

        frame_zone.setup_buffer(current_ptr, frame_size);
        current_ptr += frame_size;

        scratch_zone.setup_buffer(current_ptr, scratch_size);

        return true;
    }

    void quit_zones()
    {
        permanent_zone.reset();
        level_zone.reset();
        frame_zone.reset();
        scratch_zone.reset();
    }
}
