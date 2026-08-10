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
    concept variable_concept = std::integral<T> || std::floating_point<T>;

	struct variable
	{
		var_type type = var_type::none;
		void* val_ptr;
	};


	bool init();
	void quit();

	int printf(const char* fmt, ...);
	
	int warning(const char* fmt, ...);
	int debug(const char* fmt, ...);
	int error(const char* fmt, ...);

	bool execute(const void* cmd);
	bool register_var(void* var, var_type type);
}

