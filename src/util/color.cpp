#include "color.hpp"

#include "byteswap.hpp"
#include <charconv>

RGB_u color_from_css_hex(std::string_view s) {
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

RGB_u color_from_rgb565(u16 clr) {
	std::uint8_t r = (clr & 0xF800) >> 8;
	std::uint8_t g = (clr & 0x07E0) >> 3;
	std::uint8_t b = (clr & 0x001F) << 3;

	return {{r, g, b, 255}};
}

u16 color_to_rgb565(RGB_u clr) {
	std::uint16_t r = (clr.c.r >> 3 << 11) & 0xF800;
	std::uint16_t g = (clr.c.g >> 2 <<  5) & 0x07E0;
	std::uint16_t b = (clr.c.b >> 3)       & 0x001F;

	return r | g | b;
}
