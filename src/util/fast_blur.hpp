#pragma once
#include <cstdint>

void fast_gaussian_blur_3(std::uint8_t * in_out, std::uint8_t * scratch, const int w, const int h, const int c, const float sigma);
