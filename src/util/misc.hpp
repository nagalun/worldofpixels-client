#pragma once

#include <cstdint>
#include <cmath>
#include <cstdio>
#include <string_view>
#include <tuple>
#include <utility>
#include <string>

union twoi32 {
	struct {
		std::int32_t x;
		std::int32_t y;
	} c;
	std::uint64_t pos;
};

template<>
struct std::hash<twoi32> {
	std::size_t operator()(twoi32 s) const noexcept;
};

twoi32 mk_twoi32(std::int32_t x, std::int32_t y);
std::strong_ordering operator<=>(const twoi32& a, const twoi32& b);
bool operator==(const twoi32& a, const twoi32& b);

// square distance
std::uint32_t getDistance2dSq(twoi32 pos1, twoi32 pos2);

template<typename Coord, typename Fn>
void line(Coord x1, Coord y1, Coord x2, Coord y2, Fn& plot, bool skipfirst = true) {
	Coord dx =  std::abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	Coord dy = -std::abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	Coord err = dx + dy,
			e2;

	if (!skipfirst) { plot(x1, y1); }

	while(!(x1 == x2 && y1 == y2)) {
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x1 += sx; }
		if (e2 <= dx) { err += dx; y1 += sy; }
		plot(x1, y1);
	}
}

template<typename Fn, std::size_t... I>
void do_seq(Fn& fn, std::index_sequence<I...>) {
	fn(I...);
}

template<typename Fn, std::size_t I>
void seq(Fn& fn) {
	do_seq(fn, std::make_index_sequence<I>());
}

template <typename Tuple, std::size_t ...I, typename... Args>
static Tuple do_cartesian_make_tuple(std::index_sequence<I...>, Args&&... args) {
	std::size_t i; // to make the compiler shut up
	if constexpr (sizeof... (Args) == 0) {
		return Tuple{};
	} else if constexpr (sizeof... (Args) == 1) {
		// dumb
		return Tuple{(i=I, std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...)))...};
	} else {
		return Tuple{std::tuple_element_t<I, Tuple>{std::forward<Args>(args)...}...};
	}
}

template <typename Tuple, typename ...Args>
static Tuple cartesian_make_tuple(Args&&... args) {
    return do_cartesian_make_tuple<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple>>(), std::forward<Args>(args)...);
}

std::pair<char*, std::size_t> get_char_buf(std::size_t min_size);

template<typename... Args>
std::string_view svprintf(const char * fmt, Args... args) {
	auto [buf, sz] = get_char_buf(64);
	int written;
	while (true) {
		written = std::snprintf(buf, sz, fmt, args...);

		if (written < 0) {
			written = 0;
		} else if ((std::uint32_t)written > sz) {
			std::tie(buf, sz) = get_char_buf(written);
			continue;
		}

		break;
	}

	return std::string_view(buf, written);
}

template<typename I>
std::string n2hexstr(I w, std::size_t hex_len = sizeof(I) << 1) {
	static const char * digits = "0123456789ABCDEF";
	std::string rc(hex_len, '0');

	for (std::size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4) {
		rc[i] = digits[(w >> j) & 0x0F];
	}

	return rc;
}
