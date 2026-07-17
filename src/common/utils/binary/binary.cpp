

#include "binary.h"

#include "../../platform/memory/memory.h"

namespace tungsten::util::binary
{


	binary_cursor::binary_cursor(size_t size)
		: size{ size }
		, cursor{ 0 }
	{};


	void binary_cursor::advance(size_t count)
	{
		cursor += count;
	}

	void binary_cursor::reset_size(size_t new_size)
	{
		size = new_size;
	}

	size_t binary_cursor::tell() const
	{
		return cursor;
	}

	size_t binary_cursor::total_size() const
	{
		return size;
	}

	size_t binary_cursor::remaining() const
	{
		return total_size() - tell();
	}

	bool binary_cursor::eof() const
	{
		return remaining() == 0;
	}

	bool binary_cursor::seek(size_t offset)
	{
		if (offset > total_size())
			return false;

		cursor = offset;
		return true;
	}

	bool binary_cursor::can_advance(size_t count) const
	{
		return (remaining() >= count);
	}

	bool binary_cursor::skip(size_t count)
	{
		if (!can_advance(count))
			return false;

		advance(count);
		return true;
	}

	void binary_cursor::reset_cursor()
	{
		cursor = 0;
	}


	// ------ //
	// Reader //
	// ------ //

	binary_reader_base::binary_reader_base()
		: binary_reader_base{ 0, nullptr }
	{}

	binary_reader_base::binary_reader_base(size_t size, const void* data)
		: binary_cursor{ size }
		, data{ data }
	{}

	void binary_reader_base::reset(size_t new_size, const void* new_data)
	{
		data = new_data;
		reset_size(new_size);
		reset_cursor();
	}

	bool binary_reader_base::read_bytes(size_t count, void* dst)
	{
		if (!dst || !data)
			return false;

		if (!can_advance(count))
			return false;

		auto* src = reinterpret_cast<const uint8_t*>(data) + tell();
		platform::memory::memcpy(dst, src, count);

		advance(count);
		return true;
	}

	bool binary_reader_base::peek_bytes(size_t count, void* dst) const
	{
		if (!dst || !data)
			return false;

		if (!can_advance(count))
			return false;

		auto* src = reinterpret_cast<const uint8_t*>(data) + tell();
		platform::memory::memcpy(dst, src, count);
		return true;
	}


	const void* binary_reader_base::peak() const
	{
		return reinterpret_cast<const uint8_t*>(data) + tell();
	}

	const void* binary_reader_base::data_ptr() const
	{
		return data;
	}


	// ------ //
	// Writer //
	// ------ //


	binary_writer_base::binary_writer_base()
		: binary_writer_base{ 0, nullptr }
	{

	}
	binary_writer_base::binary_writer_base(size_t size, void* data)
		: binary_cursor{ size }
		, data{ data }
	{}

	void binary_writer_base::reset(size_t new_size, void* new_data)
	{
		data = new_data;
		reset_size(new_size);
		reset_cursor();
	}

	bool binary_writer_base::write_bytes(size_t count, const void* src)
	{
		if (!src || !data)
			return false;

		if (!can_advance(count))
			return false;

		auto* dst = reinterpret_cast<uint8_t*>(data) + tell();
		platform::memory::memcpy(dst, src, count);

		advance(count);
		return true;
	}

}