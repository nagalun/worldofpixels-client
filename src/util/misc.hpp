#pragma once

#include <cstdint>
#include <cmath>
#include <cstdio>

template<typename Coord, typename Fn>
void line(Coord x1, Coord y1, Coord x2, Coord y2, Fn plot) {
	Coord dx =  std::abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	Coord dy = -std::abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	Coord err = dx + dy,
			e2;

	while(true) {
		plot(x1, y1);
		if (x1 == x2 && y1 == y2) break;
		e2 = 2 * err;
		if (e2 >= dy) { err += dy; x1 += sx; }
		if (e2 <= dx) { err += dx; y1 += sy; }
	}
}

template <typename Tuple, std::size_t ...I, typename... Args>
static Tuple do_cartesian_make_tuple(std::index_sequence<I...>, Args&&... args) {
	if constexpr (sizeof... (Args) == 0) {
		return Tuple{};
	} else if constexpr (sizeof... (Args) == 1) {
		// dumb
		return Tuple{(I, std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...)))...};
	} else {
		return Tuple{std::tuple_element_t<I, Tuple>{std::forward<Args>(args)...}...};
	}
}

template <typename Tuple, typename ...Args>
static Tuple cartesian_make_tuple(Args&&... args) {
    return do_cartesian_make_tuple<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple>>(), std::forward<Args>(args)...);
}

template<std::size_t bufSz, typename... Args>
std::string_view svprintf(const char * fmt, Args... args) {
	static char propBuf[bufSz] = {0};
	int written = std::snprintf(propBuf, sizeof(propBuf), fmt, args...);

	if (written < 0) {
		written = 0;
	} else if ((std::uint32_t)written > sizeof(propBuf)) {
		written = sizeof(propBuf);
	}

	return std::string_view(propBuf, written);
}

