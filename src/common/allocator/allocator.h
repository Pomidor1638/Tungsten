#pragma once
#include <cstddef>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <new>
#include <cstdint>

class LinearAllocator {
public:
    explicit LinearAllocator(std::size_t size) 
    {
        allocate_block(size);
    }

    ~LinearAllocator() {
        destroy_all();
        free_block();
    }


    LinearAllocator() = delete;
    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator(LinearAllocator&&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;

    void resize(std::size_t new_size) 
    {
        destroy_all();
        free_block();
        allocate_block(new_size);
    }

    template<typename T, std::size_t count = 1, typename... Args>
    T* allocate(Args&&... args) 
    {
        constexpr bool NeedsDestructor = !std::is_trivially_destructible_v<T>;
        uintptr_t base_addr = reinterpret_cast<uintptr_t>(memory_block + offset_pointer);
        std::size_t pad_h = (alignof(DestructorRecord) - (base_addr % alignof(DestructorRecord))) % alignof(DestructorRecord);
        uintptr_t data_addr = base_addr + pad_h + (NeedsDestructor ? sizeof(DestructorRecord) : 0);
        
        std::size_t pad_d = (alignof(T) - (data_addr % alignof(T))) % alignof(T);

        T* first_ptr = reinterpret_cast<T*>(data_addr + pad_d);
        std::size_t new_offset = (reinterpret_cast<uintptr_t>(first_ptr) + sizeof(T) * count) - reinterpret_cast<uintptr_t>(memory_block);
        
        if (new_offset > hunk_size)
        {
            throw std::runtime_error(std::string(__FUNCSIG__) + "Hunk overflow!");
        }

        std::size_t old_offset = offset_pointer;
        offset_pointer = new_offset;

        try 
        {
            for (std::size_t i = 0; i < count; i++)
            {
                T* obj = &first_ptr[i];
                if constexpr (NeedsDestructor) 
                {
                    DestructorRecord* record = reinterpret_cast<DestructorRecord*>(reinterpret_cast<char*>(obj) - sizeof(DestructorRecord));
                    record->destroy = [](void* p) { static_cast<T*>(p)->~T(); };
                    record->object = obj;
                    record->prev = last_record;
                    last_record = record;
                }
                new (obj) T(std::forward<Args>(args)...);
            }
        }
        catch (...) 
        {
            throw std::runtime_error(std::string(__FUNCSIG__) + "Hunk corruption");
        }

        return first_ptr;
    }


private:
    struct DestructorRecord 
    {
        void (*destroy)(void*);
        void* object;
        DestructorRecord* prev;
    };

    void allocate_block(std::size_t size) 
    {
        hunk_size = size;
        offset_pointer = 0;
        last_record = nullptr;
        memory_block = static_cast<char*>(::operator new(size, std::align_val_t{ alignof(std::max_align_t) }));
    }

    void free_block() 
    {
        if (memory_block) 
        {
            ::operator delete(memory_block, std::align_val_t{ alignof(std::max_align_t) });
            memory_block = nullptr;
        }
    }

    void destroy_all()
    {
        while (last_record) 
        {
            last_record->destroy(last_record->object);
            last_record = last_record->prev;
        }
    }

    char* memory_block = nullptr;
    std::size_t hunk_size = 0;
    std::size_t offset_pointer = 0;
    DestructorRecord* last_record = nullptr;
};
