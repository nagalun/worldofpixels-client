#pragma once

#define bswap_64(x) __builtin_bswap64(x)
#define bswap_32(x) __builtin_bswap32(x)
#define bswap_16(x) __builtin_bswap16(x)

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#	define cnd_bswap_be_64(x) __builtin_bswap64(x)
#	define cnd_bswap_be_32(x) __builtin_bswap32(x)
#	define cnd_bswap_be_16(x) __builtin_bswap16(x)

#	define cnd_bswap_le_64(x) (x)
#	define cnd_bswap_le_32(x) (x)
#	define cnd_bswap_le_16(x) (x)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#	define cnd_bswap_be_64(x) (x)
#	define cnd_bswap_be_32(x) (x)
#	define cnd_bswap_be_16(x) (x)

#	define cnd_bswap_le_64(x) __builtin_bswap64(x)
#	define cnd_bswap_le_32(x) __builtin_bswap32(x)
#	define cnd_bswap_le_16(x) __builtin_bswap16(x)
#endif
