#include "stringparser.hpp"

#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <charconv>
#include <string>

static std::pair<char *, std::size_t> getStrNumBuf() {
	// in base 2, max is same as the amount of bits plus possible negative sign and null
	static char numBuf[2 + sizeof(std::uint64_t) * 8] = {0};
	return {numBuf, sizeof(numBuf)};
}

template<typename T, typename... Args, std::enable_if_t<!std::is_same_v<T, bool> && !std::is_floating_point_v<T>, int>>
std::optional<T> fromString(std::string_view s, Args... fmtArgs) {
	if (s.size() == 0) {
		return std::nullopt;
	}

	T n;
	auto res = std::from_chars(s.data(), s.data() + s.size(), n, fmtArgs...);

	if (res.ptr != s.data() + s.size() || res.ec != std::errc()) { // contains extra data, or too big
		return std::nullopt;
	}

	return n;
}

template<typename T, typename... Args, std::enable_if_t<!std::is_same_v<T, bool> && std::is_floating_point_v<T>, int>>
std::optional<T> fromString(std::string_view s, Args... fmtArgs) {
	static_assert(std::is_same_v<T, float> || std::is_same_v<T, double> || std::is_same_v<T, long double>,
			"not a floating point type");

	if (s.size() == 0) {
		return std::nullopt;
	}

	std::string str{s};
	char * end = nullptr;

	T n;
	if constexpr (std::is_same_v<T, float>) {
		n = std::strtof(str.c_str(), &end);
	} else if constexpr (std::is_same_v<T, double>) {
		n = std::strtod(str.c_str(), &end);
	} else if constexpr (std::is_same_v<T, long double>) {
		n = std::strtold(str.c_str(), &end);
	}

	if (end != str.c_str() + str.size()) {
		return std::nullopt;
	}

	return n;
}

template<typename T, std::enable_if_t<std::is_same_v<T, bool>, int>>
std::optional<T> fromString(std::string_view s) {
	if (s.size() != 0 && s.size() <= 5) {
		switch(s[0]) {
			case 't':
			case 'T':
				return true;

			case 'f':
			case 'F':
				return false;
		}
	}

	return std::nullopt;
}



template<typename T, typename... Args>
std::string_view toString(T v, Args... args) {
	auto b = getStrNumBuf();
	auto r = std::to_chars(b.first, b.first + b.second, v, args...);
	return std::string_view(b.first, (r.ec == std::errc() ? r.ptr : b.first) - b.first);
}



// explicit instantiations
template std::optional<std::uint8_t> fromString<std::uint8_t, int>(std::string_view, int base);
template std::optional<std::uint16_t> fromString<std::uint16_t, int>(std::string_view, int base);
template std::optional<std::uint32_t> fromString<std::uint32_t, int>(std::string_view, int base);
template std::optional<std::uint64_t> fromString<std::uint64_t, int>(std::string_view, int base);

template std::optional<std::int8_t> fromString<std::int8_t, int>(std::string_view, int base);
template std::optional<std::int16_t> fromString<std::int16_t, int>(std::string_view, int base);
template std::optional<std::int32_t> fromString<std::int32_t, int>(std::string_view, int base);
template std::optional<std::int64_t> fromString<std::int64_t, int>(std::string_view, int base);

template std::optional<std::uint8_t> fromString<std::uint8_t>(std::string_view);
template std::optional<std::uint16_t> fromString<std::uint16_t>(std::string_view);
template std::optional<std::uint32_t> fromString<std::uint32_t>(std::string_view);
template std::optional<std::uint64_t> fromString<std::uint64_t>(std::string_view);

template std::optional<std::int8_t> fromString<std::int8_t>(std::string_view);
template std::optional<std::int16_t> fromString<std::int16_t>(std::string_view);
template std::optional<std::int32_t> fromString<std::int32_t>(std::string_view);
template std::optional<std::int64_t> fromString<std::int64_t>(std::string_view);

template std::optional<bool> fromString<bool>(std::string_view);
template std::optional<float> fromString<float>(std::string_view);
template std::optional<double> fromString<double>(std::string_view);
template std::optional<long double> fromString<long double>(std::string_view);

template std::string_view toString<std::uint8_t, int>(std::uint8_t, int base);
template std::string_view toString<std::uint16_t, int>(std::uint16_t, int base);
template std::string_view toString<std::uint32_t, int>(std::uint32_t, int base);
template std::string_view toString<std::uint64_t, int>(std::uint64_t, int base);

template std::string_view toString<std::int8_t, int>(std::int8_t, int base);
template std::string_view toString<std::int16_t, int>(std::int16_t, int base);
template std::string_view toString<std::int32_t, int>(std::int32_t, int base);
template std::string_view toString<std::int64_t, int>(std::int64_t, int base);

template std::string_view toString<std::uint8_t>(std::uint8_t);
template std::string_view toString<std::uint16_t>(std::uint16_t);
template std::string_view toString<std::uint32_t>(std::uint32_t);
template std::string_view toString<std::uint64_t>(std::uint64_t);

template std::string_view toString<std::int8_t>(std::int8_t);
template std::string_view toString<std::int16_t>(std::int16_t);
template std::string_view toString<std::int32_t>(std::int32_t);
template std::string_view toString<std::int64_t>(std::int64_t);

template std::string_view toString<float>(float);
template std::string_view toString<double>(double);
template std::string_view toString<long double>(long double);
