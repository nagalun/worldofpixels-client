#pragma once

#include "explints.hpp"

#include <string_view>

union RGB_u {
	struct {
		u8 r;
		u8 g;
		u8 b;
		u8 a;
	} c;
	u32 rgb;
};

RGB_u read_css_hex_color(std::string_view);
