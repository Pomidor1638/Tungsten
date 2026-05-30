#pragma once
#include <concepts>
#include <string>

namespace tungsten::console
{
	enum class var_type
	{
		none = 0,
		boolean,
		floating,
		integer,
		string,
	};

    template <typename T>
    concept variable = std::integral<T> || std::floating_point<T>;

    class console
    {
    public:
        template <variable T>
        bool register_var(std::string varname, T* var) 
        {
            if constexpr (std::integral<T>) 
            {

            }
            return true;
        }
        template <class T>
        bool register_func(std::string funcname, T* func)
        {
        }
    private:
        
    };
}

