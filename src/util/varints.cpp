#include "varints.hpp"

#include <string>
#include <algorithm>

#if __cpp_exceptions
#	include <stdexcept>
#else
#	include <cerrno>
#endif

u64 decodeUnsignedVarint(const u8 * const data, sz_t &decodedBytes, sz_t maxBytes) {
	sz_t i = 0;
	u64 decoded_value = 0;
	sz_t shift_amount = 0;

#if !__cpp_exceptions
	// clear last error
	errno = 0;
#endif

	do {
		if (maxBytes-- == 0) {
#if __cpp_exceptions
			throw std::length_error("Varint too big!");
#else
			errno = ERANGE;
			return -1;
#endif
		}

		decoded_value |= (u64)(data[i] & 0x7F) << shift_amount;
		shift_amount += 7;
	} while ((data[i++] & 0x80) != 0);

	decodedBytes = i;
	return decoded_value;
}

sz_t encodeUnsignedVarint(u8 * const buffer, u64 value) {
	sz_t encoded = 0;

	do {
		u8 next_byte = value & 0x7F;
		value >>= 7;

		if (value) {
			next_byte |= 0x80;
		}

		buffer[encoded++] = next_byte;
	} while (value);

	return encoded;
}

sz_t unsignedVarintSize(u64 value) {
	sz_t encoded = 0;

	do {
		encoded++;
	} while (value >>= 7);

	return encoded;
}

i64 decodeSignedVarint(const u8 * const data, sz_t &decodedBytes, sz_t maxBytes) {
	u64 ur = decodeUnsignedVarint(data, decodedBytes, maxBytes);
	i64 sr = static_cast<i64>(ur >> 1);
	return (ur & 1) ? ~sr : sr;
}

sz_t encodeSignedVarint(u8 * const buffer, i64 value) {
	u64 uvalue = value < 0 ? ~value : value;
	uvalue = (uvalue << 1) | (value < 0 ? 1 : 0);
	return encodeUnsignedVarint(buffer, uvalue);
}

sz_t signedVarintSize(i64 value) {
	u64 uvalue = value < 0 ? ~value : value;
	uvalue = (uvalue << 1) | (value < 0 ? 1 : 0);
	return unsignedVarintSize(uvalue);
}

std::string getVarintString(const u8 * data, sz_t &decodedBytes) {
	sz_t lb;
	u64 size = decodeUnsignedVarint(data, lb);
	decodedBytes = lb + size;
	return std::string(reinterpret_cast<const char *>(data + lb), size);
}

sz_t setVarintString(u8 * data, std::string const& str) {
	sz_t lb = encodeUnsignedVarint(data, str.size());
	return std::copy(str.begin(), str.end(), data + lb) - data;
}

sz_t varintStringSize(std::string const& str) {
	return unsignedVarintSize(str.size()) + str.size();
}
