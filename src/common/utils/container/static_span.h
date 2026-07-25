//
//#pragma once
//
//namespace tungsten::util::container
//{
//
//    template<typename T>
//    class static_span final
//    {
//    public:
//        static_span() = default;
//
//        static_span(size_t count, T* data)
//            : count_{count}
//            , data_{data} 
//        {}
//
//        T& operator[](size_t i)
//        {
//            return data_[i];
//        }
//
//        const T& operator[](size_t i) const
//        {
//            return data_[i];
//        }
//
//        T* data()
//        {
//            return data_;
//        }
//
//        const T* data() const
//        {
//            return data_;
//        }
//
//        size_t size() const
//        {
//            return count_;
//        }
//
//        bool empty() const
//        {
//            return count_ == 0;
//        }
//
//        T* begin()
//        {
//            return data_;
//        }
//
//        T* end()
//        {
//            return data_ + count_;
//        }
//
//    private:
//        size_t  count_  = 0;
//        T*      data_   = nullptr;
//    };
//
//
//    template<>
//    class static_span<void> final
//    {
//    public:
//        static_span() = default;
//
//        static_span(size_t count, void* data)
//            : count_{count}
//            , data_{data} 
//        {}
//
//        void* data()
//        {
//            return data_;
//        }
//
//        const void* data() const
//        {
//            return data_;
//        }
//
//        size_t size() const
//        {
//            return count_;
//        }
//
//        bool empty() const
//        {
//            return count_ == 0;
//        }
//
//    private:
//        size_t  count_  = 0;
//        void*   data_   = nullptr;
//    };
//
//    template<>
//    class static_span<const void> final
//    {
//    public:
//        static_span() = default;
//
//        static_span(size_t count, const void* data)
//            : count_{count}
//            , data_{data} 
//        {}
//
//        const void* data() const
//        {
//            return data_;
//        }
//
//        size_t size() const
//        {
//            return count_;
//        }
//
//        bool empty() const
//        {
//            return count_ == 0;
//        }
//
//    private:
//        size_t      count_  = 0;
//        const void* data_   = nullptr;
//    };
//
//}