#pragma once

#include <string>
#include <compare>
#include <type_traits>

#include "util/explints.hpp"

u64 decodeUnsignedVarint(const u8 * const data, sz_t &decodedBytes, sz_t maxBytes = sizeof(u64));
sz_t encodeUnsignedVarint(u8 * const buffer, u64 value);
sz_t unsignedVarintSize(u64 value);

i64 decodeSignedVarint(const u8 * const data, sz_t &decodedBytes, sz_t maxBytes = sizeof(i64));
sz_t encodeSignedVarint(u8 * const buffer, i64 value);
sz_t signedVarintSize(i64 value);

std::string getVarintString(const u8 * const data, sz_t &decodedBytes);
sz_t setVarintString(u8 * data, std::string const&);
sz_t varintStringSize(std::string const&);

template<typename T>
class varint {
public:
	using value_type = T;

private:
	using Self = varint<value_type>;
	value_type value;

public:
	varint() noexcept { }
	varint(value_type val) noexcept
	: value(val) { }

	operator value_type() const noexcept { return value; }
	value_type get() const noexcept { return value; }

	Self operator+(value_type rhs) const noexcept { return Self(value + rhs); }
	Self operator-(value_type rhs) const noexcept { return Self(value - rhs); }
	Self operator*(value_type rhs) const noexcept { return Self(value * rhs); }
	Self operator/(value_type rhs) const noexcept { return Self(value / rhs); }
	Self operator%(value_type rhs) const noexcept { return Self(value % rhs); }

	Self& operator+=(value_type rhs) noexcept { value += rhs; return *this; }
	Self& operator-=(value_type rhs) noexcept { value -= rhs; return *this; }
	Self& operator*=(value_type rhs) noexcept { value *= rhs; return *this; }
	Self& operator/=(value_type rhs) noexcept { value /= rhs; return *this; }
	Self& operator%=(value_type rhs) noexcept { value %= rhs; return *this; }

	Self& operator++() noexcept { ++value; return *this; }
	Self& operator--() noexcept { --value; return *this; }
	Self operator++(int) noexcept { return value++; }
	Self operator--(int) noexcept { return value--; }

	std::strong_ordering operator<=>(Self rhs) const noexcept { return value <=> rhs.value; }
	std::strong_ordering operator<=>(value_type rhs) const noexcept { return value <=> rhs; }

	static Self read(const u8 * data, sz_t &decodedBytes, sz_t maxBytes = sizeof(value_type)) {
		if constexpr (std::is_signed_v<value_type>) {
			return decodeSignedVarint(data, decodedBytes, maxBytes);
		} else {
			return decodeUnsignedVarint(data, decodedBytes, maxBytes);
		}
	}

	sz_t write(u8 * data) {
		if constexpr (std::is_signed_v<value_type>) {
			return encodeSignedVarint(data, value);
		} else {
			return encodeUnsignedVarint(data, value);
		}
	}

	sz_t size() {
		if constexpr (std::is_signed_v<value_type>) {
			return signedVarintSize(value);
		} else {
			return unsignedVarintSize(value);
		}
	}
};

using ivar = varint<i64>;
using uvar = varint<u64>;
