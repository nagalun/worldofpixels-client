#pragma once

#include <string>

#include "explints.hpp"

u64 decodeUnsignedVarint(const u8 * const data, sz_t &decodedBytes, sz_t maxBytes = sizeof(u64));
sz_t encodeUnsignedVarint(u8 * const buffer, u64 value);
sz_t unsignedVarintSize(u64 value);

std::string getVarintString(const u8 * const data, sz_t &decodedBytes);
sz_t setVarintString(u8 * data, std::string const&);
sz_t varintStringSize(std::string const&);
