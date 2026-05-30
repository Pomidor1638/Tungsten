#pragma once
#include <cstddef>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <new>
#include <cstdint>
#include <limits>

namespace tungsten::util::allocator
{
    class LinearAllocator final
    {
    public:

        struct Marker
        {
            size_t offset_pointer = 0;
            void*  last_record    = nullptr;
            size_t generation     = 0;
        };

        explicit LinearAllocator()
        {
            destroy_all();
        }

        explicit LinearAllocator(std::size_t size)
        {
            destroy_all();
            allocate_block(size);
        }

        ~LinearAllocator() 
        {
            destroy_all();
            free_block();
        }


        LinearAllocator(const LinearAllocator&) = delete;
        LinearAllocator(LinearAllocator&&) = delete;
        LinearAllocator& operator=(const LinearAllocator&) = delete;

        void resize(std::size_t new_size) 
        {
            destroy_all();
            free_block();
            allocate_block(new_size);
        }

        Marker mark() const
        {
            Marker marker{};
            marker.offset_pointer = offset_pointer;
            marker.last_record    = last_record;
            marker.generation     = generation;

            return marker;
        }

        void rollback(Marker marker)
        {
            validate_marker(marker);

            DestructorRecord* marker_record = static_cast<DestructorRecord*>(marker.last_record);

            while (last_record != marker_record)
            {
                if (!last_record)
                    throw std::runtime_error("LinearAllocator rollback corruption: marker record was not found");

                const size_t record_offset = offset_from_pointer(last_record);

                if (record_offset < marker.offset_pointer)
                    throw std::runtime_error("LinearAllocator rollback corruption: destructor record is before marker");

                DestructorRecord* record = last_record;
                DestructorRecord* previous = record->previous;

                destroy_record(record);

                last_record = previous;
            }

            offset_pointer = marker.offset_pointer;
        }

        void reset()
        {
            Marker root{};
            root.generation = generation;

            rollback(root);
        }

        size_t used() const
        {
            return offset_pointer;
        }

        size_t capacity() const
        {
            return hunk_size;
        }

        template<typename T, size_t count, typename... Args>
        T* allocate(Args&&... args)
        {
            static_assert(count > 0, "LinearAllocator allocation count must be greater than zero");

            const size_t object_size      = sizeof(T);
            const size_t object_alignment = alignof(T);

            if (count > std::numeric_limits<size_t>::max() / object_size)
                throw std::runtime_error("LinearAllocator allocation size overflow");

            const size_t object_padding = align_padding(offset_pointer, object_alignment);

            const size_t object_offset = offset_pointer + object_padding;
            const size_t objects_size  = object_size * count;

            if (object_offset < offset_pointer)
                throw std::runtime_error("LinearAllocator allocation offset overflow");

            const size_t after_objects = object_offset + objects_size;

            if (after_objects < object_offset)
                throw std::runtime_error("LinearAllocator allocation offset overflow");

            size_t record_offset = after_objects;
            size_t total_size = after_objects - offset_pointer;

            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                const size_t record_size      = sizeof (DestructorRecord);
                const size_t record_alignment = alignof(DestructorRecord);

                const size_t record_padding = align_padding(after_objects, record_alignment);

                record_offset = after_objects + record_padding;

                if (record_offset < after_objects)
                    throw std::runtime_error("LinearAllocator allocation record offset overflow");

                total_size = record_offset + record_size - offset_pointer;
            }

            if (offset_pointer > hunk_size || total_size > (hunk_size - offset_pointer))
                throw std::runtime_error("zone overflow");

            const Marker allocation_marker = mark();
            T* result = reinterpret_cast<T*>(memory_block + object_offset);

            DestructorRecord* record = nullptr;

            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                record = reinterpret_cast<DestructorRecord*>(memory_block + record_offset);

                new (record) DestructorRecord{};

                record->destroy = &destroy_one<T>;
                record->object = result;
                record->count = 0;
                record->size_of = object_size;
                record->previous = last_record;

                last_record = record;
            }

            // Reserve this block before construction. Constructors may allocate from
            // this allocator again, and nested allocations must go after this block.
            offset_pointer += total_size;

            size_t constructed = 0;

            try
            {
                for (; constructed < count; constructed++)
                {
                    new (&result[constructed]) T(std::forward<Args>(args)...);

                    if constexpr (!std::is_trivially_destructible_v<T>)
                        record->count = constructed + 1;
                }
            }
            catch (...)
            {
                rollback(allocation_marker);
                throw;
            }

            return result;
        }


    private:


        template<typename T>
        static void destroy_one(void* object)
        {
            static_cast<T*>(object)->~T();
        }

        struct DestructorRecord 
        {
            void (*destroy)(void*);
            void* object;

            size_t count;
            size_t size_of;
            DestructorRecord* previous;
        };

        uint8_t* memory_block           = nullptr;
        size_t   hunk_size              = 0;
        size_t   offset_pointer         = 0;
        size_t   generation             = 0;
        DestructorRecord* last_record   = nullptr;

        /*

            <         lower address>
        data_frame:
            <             some data> <---- aligned
            <      DestructorRecord>

        next_frame:
            <        **next frame**> <---- offset_pointer points to the next free zone

            ... etc

            <        higher address> 

        */

        void allocate_block(std::size_t size) 
        {
            hunk_size             = size;
            offset_pointer        = 0;
            last_record           = nullptr;
            generation++;
            memory_block          = static_cast<uint8_t*>(::operator new(size, std::align_val_t{ alignof(std::max_align_t) }));
        }

        void free_block() 
        {
            if (memory_block) 
            {
                ::operator delete(memory_block, std::align_val_t{ alignof(std::max_align_t) });
                memory_block = nullptr;
            }

            hunk_size      = 0;
            offset_pointer = 0;
            last_record    = nullptr;
        }

        void destroy_all()
        {
            reset();
        }

        size_t align_padding(size_t offset, size_t alignment) const
        {
            const uintptr_t address = reinterpret_cast<uintptr_t>(memory_block) + offset;
            return (alignment - (address % alignment)) % alignment;
        }

        size_t offset_from_pointer(const void* pointer) const
        {
            const uintptr_t base = reinterpret_cast<uintptr_t>(memory_block);
            const uintptr_t address = reinterpret_cast<uintptr_t>(pointer);

            if (address < base)
                throw std::runtime_error("LinearAllocator pointer is before memory block");

            return static_cast<size_t>(address - base);
        }

        void validate_marker(Marker marker) const
        {
            if (marker.generation != generation)
                throw std::runtime_error("LinearAllocator marker belongs to another allocator generation");

            if (marker.offset_pointer > offset_pointer)
                throw std::runtime_error("LinearAllocator marker offset is greater than current offset");

            if (marker.last_record)
                validate_record_pointer(static_cast<const DestructorRecord*>(marker.last_record));
        }

        void validate_record_pointer(const DestructorRecord* record) const
        {
            const uintptr_t base    = reinterpret_cast<uintptr_t>(memory_block);
            const uintptr_t end     = base + hunk_size;
            const uintptr_t address = reinterpret_cast<uintptr_t>(record);

            if (address < base || address + sizeof(DestructorRecord) > end)
                throw std::runtime_error("LinearAllocator destructor record is outside of memory block");

            if (address % alignof(DestructorRecord) != 0)
                throw std::runtime_error("LinearAllocator destructor record alignment is corrupted");
        }

        void destroy_record(DestructorRecord* record)
        {
            validate_record_pointer(record);

            if (!record->destroy)
                throw std::runtime_error("LinearAllocator destructor record has no destroy function");

            uint8_t* object_bytes = static_cast<uint8_t*>(record->object);

            for (size_t i = record->count; i > 0; i--)
            {
                void* object = object_bytes + (i - 1) * record->size_of;
                record->destroy(object);
            }
        }

    };
}
