

#pragma once
#include "../../platform/endian/endian.h"

namespace tungsten::util::binary
{


	enum class bin_endian_type
	{
		big,
		little,
		native
	};

	template <bin_endian_type type, typename T>
	T bin_convert(T value)
	{
		if constexpr (type == bin_endian_type::big)
		{
			return platform::endian::to_big(value);
		}
		else if constexpr (type == bin_endian_type::little)
		{
			return platform::endian::to_little(value);
		}
		else
		{
			return value;
		}
	}

	// implemantation

	class binary_cursor
	{
	public:
		binary_cursor(size_t size);
		~binary_cursor() = default;

		size_t tell() const;
		size_t total_size() const;
		size_t remaining() const;
		bool eof() const;

		bool seek(size_t offset);
		bool skip(size_t count);

		bool can_advance(size_t count) const;
		void reset_cursor();

	protected:

		void advance(size_t count);

	private:
		size_t cursor = 0;
		size_t size = 0;
	};

	class binary_reader_base : public binary_cursor
	{
	public:
		binary_reader_base();
		binary_reader_base(size_t size, const void* data);


		bool read_bytes(size_t count, void* dst);
		bool peek_bytes(size_t count, void* dst) const;
		
		const void* peak() const;
		const void* data_ptr() const;

	private:
		const void* data = nullptr;
	};

	template <bin_endian_type type>
	class binary_reader final : public binary_reader_base
	{
	public:

		using binary_reader_base::binary_reader_base;

		template <typename T>
		bool read(T& value)
		{
			constexpr size_t size = sizeof(T);

			if (!read_bytes(size, &value))
				return false;

			value = bin_convert<type>(value);
			return true;
		}
	private:

	};

	class binary_writer_base : public binary_cursor
	{
	public:

		binary_writer_base();
		binary_writer_base(size_t size, void* data);

		bool write_bytes(size_t count, const void* src);


	private:
		void* data = nullptr;
	};

	template <bin_endian_type type>
	class binary_writer final : public binary_writer_base
	{
	public:

		using binary_writer_base::binary_writer_base;

		template <typename T>
		bool write(const T& value)
		{
			constexpr size_t size = sizeof(T);
			T protocol_value = bin_convert<type>(value);
			return write_bytes(size, &protocol_value);
		}
	};
}