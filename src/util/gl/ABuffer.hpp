#pragma once

#include "Buffer.hpp"

#include <cstdint>
#include <cstddef>
#include <initializer_list>

namespace gl {

class ABuffer : public Buffer {
public:
	ABuffer() = default;
	ABuffer(std::initializer_list<float> verts);
	ABuffer(std::initializer_list<float> verts, std::uint32_t usage);

	void use() const;
	void data(std::size_t size, const void * data, std::uint32_t usage);
};

} /* namespace gl */
