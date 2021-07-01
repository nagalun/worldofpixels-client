#pragma once

#include <cstdint>

namespace gl {

class VtxArray {
	std::uint32_t id;

public:
	VtxArray();
	~VtxArray();

	VtxArray(VtxArray &&other);
	VtxArray& operator=(VtxArray &&other);
	VtxArray(const VtxArray &other) = delete;
	VtxArray& operator=(const VtxArray &other) = delete;

	void use() const;
	std::uint32_t get() const;
	void enableAttribs(std::uint32_t n);

private:
	void del();
};

} /* namespace gl */
