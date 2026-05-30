#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <utility>

namespace tungsten::util::container
{
    template <typename T, std::size_t MaxSize>
    class ring_queue
    {
    public:
        static_assert(MaxSize > 0, "ring_queue capacity must be greater than zero");

        bool push(const T& value)
        {
            if (full())
                return false;

            items[tail] = value;
            tail = next(tail);
            ++count;
            return true;
        }

        bool push(T&& value)
        {
            if (full())
                return false;

            items[tail] = std::move(value);
            tail = next(tail);
            ++count;
            return true;
        }

        bool pop(T& value)
        {
            if (empty())
                return false;

            value = std::move(items[head]);
            head = next(head);
            --count;
            return true;
        }

        T& front()
        {
            assert(!empty());
            return items[head];
        }

        const T& front() const
        {
            assert(!empty());
            return items[head];
        }

        void clear()
        {
            head = 0;
            tail = 0;
            count = 0;
        }

        bool empty() const
        {
            return count == 0;
        }

        bool full() const
        {
            return count == MaxSize;
        }

        std::size_t size() const
        {
            return count;
        }

        static constexpr std::size_t capacity()
        {
            return MaxSize;
        }

    private:
        static constexpr std::size_t next(std::size_t index)
        {
            return (index + 1) % MaxSize;
        }

        std::array<T, MaxSize> items{};
        std::size_t head = 0;
        std::size_t tail = 0;
        std::size_t count = 0;
    };
}
