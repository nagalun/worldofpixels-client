#pragma once

#include <cstdint>
#include <cstddef>

namespace gl {

class Framebuffer {
	std::uint32_t id;

public:
	Framebuffer();
	Framebuffer(std::nullptr_t);
	~Framebuffer();

	Framebuffer(Framebuffer &&other);
	Framebuffer& operator=(Framebuffer &&other);
	Framebuffer(const Framebuffer &other) = delete;
	Framebuffer& operator=(const Framebuffer &other) = delete;

	void use(std::uint32_t type) const;
	std::uint32_t get() const;

private:
	void del();
};

} /* namespace gl */
