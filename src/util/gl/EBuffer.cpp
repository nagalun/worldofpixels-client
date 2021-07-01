#include "EBuffer.hpp"

#include <GLES2/gl2.h>

namespace gl {

EBuffer::EBuffer(std::initializer_list<float> verts)
: EBuffer(verts, GL_STATIC_DRAW) { }

EBuffer::EBuffer(std::initializer_list<float> verts, std::uint32_t usage) {
	data(verts.size() * sizeof(float), verts.begin(), usage);
}

void EBuffer::use() const {
	Buffer::use(GL_ELEMENT_ARRAY_BUFFER);
}

void EBuffer::data(std::size_t size, const void *data, std::uint32_t usage) {
	Buffer::data(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}

} /* namespace gl */
