#include "Texture.hpp"

#include <utility>
#include <GLES2/gl2.h>

namespace gl {

Texture::Texture() {
	glGenTextures(1, &id);
}

Texture::Texture(std::nullptr_t)
: id(0) { }

Texture::~Texture() {
	del();
}

Texture::Texture(Texture&& b)
: id(std::exchange(b.id, 0)) { }

Texture& Texture::operator=(Texture&& b) {
	del();
	id = std::exchange(b.id, 0);
	return *this;
}

void Texture::use(std::uint32_t type) const {
	glBindTexture(type, id);
}

std::uint32_t Texture::get() const {
	return id;
}

void Texture::del() {
	if (id != 0) {
		glDeleteTextures(1, &id);
	}
}

} /* namespace gl */
