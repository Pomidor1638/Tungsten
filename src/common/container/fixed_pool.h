#pragma once


#include <cstddef>
#include <bitset>
#include <type_traits>
#include <utility>
#include <new>

namespace tungsten::container
{
    template<class T, size_t pool_capacity>
    class fixed_pool final
    {
    public:

        using size_type         = size_t;
        using index_type        = size_t;
        
        static constexpr index_type nosize = index_type(-1);
        
        using value_type        = T;

        using reference         = value_type&;
        using const_reference   = const value_type&;        

        using pointer           = value_type*;
        using const_pointer     = const value_type*;


        class iterator 
        {
        public:

            friend class fixed_pool;

            iterator(const iterator&) = default;
            iterator(iterator&&) = default;
            ~iterator() = default;



            reference operator*()
            {
                return *pool->raw_ptr(idx);
            }
            pointer operator->()
            {
                return pool->raw_ptr(idx);
            }

            const_reference operator*() const
            {
                return *pool->raw_ptr(idx);
            }
            const_pointer operator->() const
            {
                return pool->raw_ptr(idx);
            }

            bool operator==(const iterator& o) const
            {
                return idx == o.idx && pool == o.pool;
            }
            bool operator!=(const iterator& o) const
            {
                return idx != o.idx || pool != o.pool;
            }
            iterator& operator++()
            {
                idx = pool->find_next_alive<true>(idx+1);
                return *this;
            }
            /*
            iterator& operator--()
            {
                idx = pool->find_next_alive<false>(idx-1);
                return *this;
            }
            */
        private:

            iterator(fixed_pool<value_type, pool_capacity>* p, index_type i)
                : pool(p)
                , idx(i)
            {
            }

            fixed_pool<value_type, pool_capacity>* pool = nullptr;
            index_type idx = nosize;
        };

        iterator&& begin()
        {
            return iterator{this, find_next_alive<true>(0)};
        }
        iterator&& end()
        {
            return iterator{this, nosize};
        }

        const iterator& cbegin() const
        {
            return iterator{this, find_next_alive<true>(0)};
        }
        const iterator& cend() const
        {
            return iterator{this, nosize};
        }

        using const_iterator    = const iterator;
        using difference_type   = ptrdiff_t;

        fixed_pool()                     = default;
        fixed_pool(const fixed_pool&  o) = delete;
        fixed_pool(      fixed_pool&& o) = delete;

        fixed_pool& operator=(const fixed_pool&  o) = delete;
        fixed_pool& operator=(      fixed_pool&& o) = delete;
 
        ~fixed_pool()
        {
            clear();    
        }

        pointer get(index_type idx)
        {
            return valid(idx) ? reinterpret_cast<pointer>(&slots[idx].data) : nullptr;
        }

        const_pointer get(index_type idx) const
        {
            return valid(idx) ? reinterpret_cast<const_pointer>(&slots[idx].data) : nullptr;
        }
        
        size_type size() const 
        {
            return valid_count;
        }

        constexpr size_type capacity() const
        {
            return pool_capacity;
        }
        
        bool empty() const
        {
            return valid_count == 0;
        }

        bool full() const
        {
            return valid_count == pool_capacity; 
        }

        bool valid(index_type idx) const
        {
            if (idx < pool_capacity)
                return valid_mask[idx];
            return false;
        }

        template<class ...Args>
        bool emplace(index_type& out_idx, Args&&...args)
        {
            out_idx = nosize;
            if (full() || valid(next_empty))
                return false;


            out_idx = next_empty;
            pointer p = raw_ptr(out_idx);
            ::new(p) value_type(std::forward<Args>(args)...);

            valid_mask[out_idx] = true;
            valid_count++;
            next_empty = find_empty();
            
            return true;
        }

        bool remove(index_type idx)
        {
            if (empty() || !valid(idx))
                return false;
            
            destroy_slot(idx);
            update();

            return true;
        }

        void clear()
        {
            for (index_type i = 0; (i < pool_capacity) && valid_count; i++)
            {
                if (valid_mask[i])
                {
                    destroy_slot(i);
                }
            }

            next_empty = 0;
            valid_count = 0;
            valid_mask = {};
        }
        
    private:

        struct raw_slot
        {
            alignas(value_type) unsigned char data[sizeof(value_type)];
        };

        index_type next_empty  = 0;
        index_type valid_count = 0;

        raw_slot slots[pool_capacity]{};
        std::bitset<pool_capacity> valid_mask{}; 

        pointer raw_ptr(index_type idx)
        {
            return reinterpret_cast<pointer>(&slots[idx].data);
        }

        void destroy_slot(index_type idx)
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                pointer p = raw_ptr(idx);
                p->~value_type(); 
            }
            valid_count--;
            valid_mask[idx] = false;
        }

        index_type find_empty() const
        {
            for (index_type i = 0; i < pool_capacity; i++)
            {
                if (!valid_mask[i])
                    return i; 
            }        
            return nosize;
        }

        
        template<bool direction>
        index_type find_next_alive(index_type s)
        {
            if (!valid_count)
                return nosize;

            while (s < pool_capacity && !valid_mask[s]) 
            {
                if constexpr (direction) 
                {
                    s++;
                }
                else
                {
                    s--;
                }
            }

            if (s >= pool_capacity)
                s = nosize;

            return s;
        }

        void update()
        {
            next_empty = find_empty();
        }
    };
}