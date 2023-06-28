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

RGB_u color_from_css_hex(std::string_view);
RGB_u color_from_rgb565(u16 clr);
u16 color_to_rgb565(RGB_u clr);
