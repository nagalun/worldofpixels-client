#pragma once

#include <cstdint>

#include "explints.hpp"

union RGB_u {
	struct {
		u8 r;
		u8 g;
		u8 b;
		u8 a;
	} c;
	u32 rgb;
};
