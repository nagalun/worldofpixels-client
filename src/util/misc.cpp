#include "misc.hpp"
#include <memory>

std::size_t std::hash<twoi32>::operator()(twoi32 s) const noexcept {
	return std::hash<std::uint64_t>{}(s.pos);
}

std::strong_ordering operator<=>(const twoi32& a, const twoi32& b) {
	return a.pos <=> b.pos;
}

bool operator==(const twoi32& a, const twoi32& b) {
	return a.pos == b.pos;
}

twoi32 mk_twoi32(std::int32_t x, std::int32_t y) {
	twoi32 u;
	u.c.x = x;
	u.c.y = y;

	return u;
}

// square distance
std::uint32_t getDistance2dSq(twoi32 pos1, twoi32 pos2) {
	std::uint32_t dxr = std::abs(pos2.c.x - pos1.c.x);
	std::uint32_t dyr = std::abs(pos2.c.y - pos1.c.y);
	return std::max(dxr, dyr);
}

std::pair<char*, std::size_t> get_char_buf(std::size_t min_size) {
	static std::unique_ptr<char[]> buf = nullptr;
	static std::size_t size = 0;

	if (size < min_size) {
		buf = std::make_unique<char[]>(min_size);
		size = min_size;
	}

	return {buf.get(), size};
}
