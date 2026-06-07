
#pragma once

namespace tungsten::util::container
{

    template<typename T>
    class static_span
    {
    public:
        static_span() = default;

        static_span(T* data, size_t count)
            : data_(data), count_(count)
        {}

        T& operator[](size_t i)
        {
            return data_[i];
        }

        const T& operator[](size_t i) const
        {
            return data_[i];
        }

        T* data()
        {
            return data_;
        }

        const T* data() const
        {
            return data_;
        }

        size_t size() const
        {
            return count_;
        }

        bool empty() const
        {
            return count_ == 0;
        }

        T* begin()
        {
            return data_;
        }

        T* end()
        {
            return data_ + count_;
        }

    private:
        T* data_ = nullptr;
        size_t count_ = 0;
    };

}