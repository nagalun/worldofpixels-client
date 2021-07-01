#include "ABuffer.hpp"

#include <GLES2/gl2.h>

namespace gl {

ABuffer::ABuffer(std::initializer_list<float> verts)
: ABuffer(verts, GL_STATIC_DRAW) { }

ABuffer::ABuffer(std::initializer_list<float> verts, std::uint32_t usage) {
	data(verts.size() * sizeof(float), verts.begin(), usage);
}

void ABuffer::use() const {
	Buffer::use(GL_ARRAY_BUFFER);
}

void ABuffer::data(std::size_t size, const void *data, std::uint32_t usage) {
	Buffer::data(GL_ARRAY_BUFFER, size, data, usage);
}

} /* namespace gl */
