#include "Framebuffer.hpp"

#include <utility>
#include <GLES2/gl2.h>

namespace gl {

Framebuffer::Framebuffer() {
	glGenFramebuffers(1, &id);
}

Framebuffer::Framebuffer(std::nullptr_t)
: id(0) { }

Framebuffer::~Framebuffer() {
	del();
}

Framebuffer::Framebuffer(Framebuffer&& b)
: id(std::exchange(b.id, 0)) { }

Framebuffer& Framebuffer::operator=(Framebuffer&& b) {
	del();
	id = std::exchange(b.id, 0);
	return *this;
}

void Framebuffer::use(std::uint32_t type) const {
	glBindFramebuffer(type, id);
}

std::uint32_t Framebuffer::get() const {
	return id;
}

void Framebuffer::del() {
	if (id != 0) {
		glDeleteFramebuffers(1, &id);
	}
}

} /* namespace gl */
