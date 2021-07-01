#include "Buffer.hpp"

#include <utility>
#include <GLES2/gl2.h>

namespace gl {

Buffer::Buffer() {
	glGenBuffers(1, &id);
}

Buffer::~Buffer() {
	del();
}

Buffer::Buffer(Buffer&& b)
: id(std::exchange(b.id, 0)) { }

Buffer& Buffer::operator=(Buffer&& b) {
	del();
	id = std::exchange(b.id, 0);
	return *this;
}

void Buffer::use(std::uint32_t type) const {
	glBindBuffer(type, id);
}

void Buffer::data(std::uint32_t type, std::size_t size, const void *data,
		std::uint32_t usage) {
	use(type);
	glBufferData(type, size, data, usage);
}

std::uint32_t Buffer::get() const {
	return id;
}

void Buffer::del() {
	if (id != 0) {
		glDeleteBuffers(1, &id);
	}
}

} /* namespace gl */
