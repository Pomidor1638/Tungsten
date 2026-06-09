
#pragma once
#include <cstdint>

namespace tungsten::util::str 
{
    template <int N>
	class fixed_string final
	{
    public:

        fixed_string() = default;
        fixed_string(const fixed_string&) = default;
        fixed_string(fixed_string&&) = default;

        fixed_string(const char* str)
            : size{0}
            , data{}
        {
            if (!str)
                return;
         
            while (*str && size < capacity())
            {
                data[size] = str++;
                size++;
            }
        }

        ~fixed_string() = default;
    
        char& operator[](size_t k)
        {
            return data[k];
        }

        char operator[](size_t k) const
        {
            if (k >= size || k >= capacity())
            {
                return '\0';
            }

            return data[k];
        }

        const char* c_str() const
        {
            return data;
        }

        size_t length() const
        {
            return size;
        }

        size_t constexpr capacity() const
        {
            return N;
        }

        char* begin()
        {
            return data;
        }

        char* end()
        {
            return data + size;
        }

        const char* cbegin() const
        {
            return data;
        }

        const char* cend() const
        {
            return data + size;
        }

    private:
		size_t	size = 0;
		char 	data[N]{};
	};




    template <int N>
	class fixed_wstring final
	{
    public:

        fixed_wstring() = default;
        fixed_wstring(const fixed_wstring&) = default;
        fixed_wstring(fixed_wstring&&) = default;

        fixed_wstring(const wchar_t* str)
            : size{0}
            , data{}
        {
            if (!str)
                return;
         
            while (*str && size < capacity())
            {
                data[size] = str++;
                size++;
            }
        }

        ~fixed_wstring() = default;
    
        wchar_t& operator[](size_t k)
        {
            return data[k];
        }

        wchar_t operator[](size_t k) const
        {
            if (k >= size || k >= capacity())
            {
                return '\0';
            }

            return data[k];
        }

        const wchar_t* c_str() const
        {
            return data;
        }

        size_t length() const
        {
            return size;
        }

        size_t constexpr capacity() const
        {
            return N;
        }

        wchar_t* begin()
        {
            return data;
        }

        wchar_t* end()
        {
            return data + size;
        }

        const wchar_t* cbegin() const
        {
            return data;
        }

        const wchar_t* cend() const
        {
            return data + size;
        }

    private:
		size_t  size = 0;
		wchar_t data[N]{};
	};

}