#pragma once

#include <cstdint>
#include <cstddef>

namespace gl {

class Texture {
	std::uint32_t id;

public:
	Texture();
	Texture(std::nullptr_t);
	~Texture();

	Texture(Texture &&other);
	Texture& operator=(Texture &&other);
	Texture(const Texture &other) = delete;
	Texture& operator=(const Texture &other) = delete;

	void use(std::uint32_t type) const;
	std::uint32_t get() const;

private:
	void del();
};

} /* namespace gl */
