#include "binary.h"
#include "../../platform/memory/memory.h"

namespace tungsten::util::binary
{
	binary_cursor::binary_cursor(size_t size)
		: cursor_{ 0 }
		, size_{ size }
	{}

	void binary_cursor::advance(size_t count)
	{
		cursor_ += count;
	}

	size_t binary_cursor::tell() const
	{
		return cursor_;
	}

	size_t binary_cursor::total_size() const
	{
		return size_;
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

		cursor_ = offset;
		return true;
	}

	bool binary_cursor::can_advance(size_t count) const
	{
		return remaining() >= count;
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
		cursor_ = 0;
	}

}