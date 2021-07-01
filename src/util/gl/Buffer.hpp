#pragma once

#include <cstdint>
#include <cstddef>

namespace gl {

class Buffer {
	std::uint32_t id;

public:
	Buffer();
	~Buffer();

	Buffer(Buffer &&other);
	Buffer& operator=(Buffer &&other);
	Buffer(const Buffer &other) = delete;
	Buffer& operator=(const Buffer &other) = delete;

	void use(std::uint32_t type) const;
	void data(std::uint32_t type, std::size_t size, const void * data, std::uint32_t usage);
	std::uint32_t get() const;

private:
	void del();
};

} /* namespace gl */
