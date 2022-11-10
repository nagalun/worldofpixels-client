#include "color.hpp"

#include "byteswap.hpp"
#include <charconv>

RGB_u read_css_hex_color(std::string_view s) {
	RGB_u newClr;

	// + 1 to skip '#'
	auto res = std::from_chars<u32>(s.data() + 1, s.data() + s.size(), newClr.rgb, 16);
	if (res.ec == std::errc::invalid_argument || res.ec == std::errc::result_out_of_range) {
		return {{0, 0, 0, 255}};
	}

	if (s.size() == 6 + 1) { // if format is #rrggbb, correct alpha value
		newClr.rgb <<= 8;
		newClr.rgb |= 0xFF;
	}

	newClr.rgb = bswap_32(newClr.rgb);

	return newClr;
}
