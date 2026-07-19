#pragma once
#include "../../platform/endian/endian.h"
#include "../../platform/memory/memory.h"

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
			return platform::endian::to_big(value);
		else if constexpr (type == bin_endian_type::little)
			return platform::endian::to_little(value);
		else
			return value;
	}

	class binary_cursor
	{
	public:
		explicit binary_cursor(size_t size);
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
		size_t cursor_ = 0;
		size_t size_ = 0;
	};

	class binary_reader_base
	{
	public:
		virtual ~binary_reader_base() = default;

		virtual bool read_bytes(size_t count, void* dst) = 0;
		virtual bool peek_bytes(size_t count, void* dst) const = 0;

		virtual size_t tell() const = 0;
		virtual size_t total_size() const = 0;
		virtual size_t remaining() const = 0;
		virtual bool eof() const = 0;

		virtual bool seek(size_t offset) = 0;
		virtual bool skip(size_t count) = 0;
	};

	template <bin_endian_type type>
	class binary_reader : public binary_reader_base
	{
	public:
		template <typename T>
		bool read(T& value)
		{
			if (!read_bytes(sizeof(T), &value))
				return false;

			value = bin_convert<type>(value);
			return true;
		}
	};

	class binary_writer_base
	{
	public:
		virtual ~binary_writer_base() = default;

		virtual bool write_bytes(size_t count, const void* src) = 0;

		virtual size_t tell() const = 0;
		virtual size_t total_size() const = 0;
		virtual size_t remaining() const = 0;
		virtual bool eof() const = 0;

		virtual bool seek(size_t offset) = 0;
		virtual bool skip(size_t count) = 0;
	};

	template <bin_endian_type type>
	class binary_writer : public binary_writer_base
	{
	public:
		template <typename T>
		bool write(const T& value)
		{
			T converted = bin_convert<type>(value);
			return write_bytes(sizeof(T), &converted);
		}
	};

	template <bin_endian_type type>
	class memory_reader final : public binary_reader<type>
	{
	public:
		memory_reader(size_t size, const void* data)
			: cursor{ size }, data{ data } {}

		bool read_bytes(size_t count, void* dst) override
		{
			if (!dst || !data || !cursor.can_advance(count))
				return false;

			auto* src = static_cast<const uint8_t*>(data) + cursor.tell();
			platform::memory::memcpy(dst, src, count);

			cursor.advance(count);
			return true;
		}
		bool peek_bytes(size_t count, void* dst) const override
		{
			if (!dst || !data || !cursor.can_advance(count))
				return false;

			auto* src = static_cast<const uint8_t*>(data) + cursor.tell();
			platform::memory::memcpy(dst, src, count);
			return true;
		}

		size_t tell() const override { return cursor.tell(); }
		size_t total_size() const override { return cursor.total_size(); }
		size_t remaining() const override { return cursor.remaining(); }
		bool eof() const override { return cursor.eof(); }
		bool seek(size_t offset) override { return cursor.seek(offset); }
		bool skip(size_t count) override { return cursor.skip(count); }

		const void* peek() const
		{
			return static_cast<const uint8_t*>(data) + cursor.tell();
		}

		const void* data_ptr() const { return data; }

	private:
		binary_cursor cursor;
		const void* data = nullptr;
	};


	template <bin_endian_type type>
	class memory_writer final : public binary_writer<type>
	{
	public:
		memory_writer(size_t size, void* data)
			: cursor{ size }, data{ data } {}

		bool write_bytes(size_t count, const void* src) override
		{
			if (!src || !data || !cursor.can_advance(count))
				return false;

			auto* dst = static_cast<uint8_t*>(data) + cursor.tell();
			platform::memory::memcpy(dst, src, count);

			cursor.advance(count);
			return true;
		}

		size_t tell() const override { return cursor.tell(); }
		size_t total_size() const override { return cursor.total_size(); }
		size_t remaining() const override { return cursor.remaining(); }
		bool eof() const override { return cursor.eof(); }
		bool seek(size_t offset) override { return cursor.seek(offset); }
		bool skip(size_t count) override { return cursor.skip(count); }

		void* peek()
		{
			return static_cast<uint8_t*>(data) + cursor.tell();
		}

		void* data_ptr() { return data; }

	private:
		binary_cursor cursor;
		void* data = nullptr;
	};
}