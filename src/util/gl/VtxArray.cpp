#include "VtxArray.hpp"

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <utility>

namespace gl {

VtxArray::VtxArray() {
	glGenVertexArraysOES(1, &id);
}

VtxArray::~VtxArray() {
	del();
}

VtxArray::VtxArray(VtxArray&& a)
: id(std::exchange(a.id, 0)) { }

VtxArray& VtxArray::operator=(VtxArray&& a) {
	del();
	id = std::exchange(a.id, 0);
	return *this;
}

void VtxArray::use() const {
	glBindVertexArrayOES(id);
}

std::uint32_t VtxArray::get() const {
	return id;
}

void VtxArray::enableAttribs(std::uint32_t n) {
	use();

	for (std::uint32_t i = 0; i < n; i++) {
		glEnableVertexAttribArray(i);
	}
}

void VtxArray::del() {
	if (id != 0) {
		glDeleteVertexArraysOES(1, &id);
	}
}

} /* namespace gl */
