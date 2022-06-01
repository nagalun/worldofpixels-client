#pragma once

#include <array>
#include <util/explints.hpp>

// to be able to embed this value on string literals
#define CHUNK_CONSTANTS_SIZE 512

constexpr u8 popc(u32 n) {
	n = (n & 0x55555555) + ((n >> 1) & 0x55555555);
	n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
	n = (n & 0x0F0F0F0F) + ((n >> 4) & 0x0F0F0F0F);
	n = (n & 0x00FF00FF) + ((n >> 8) & 0x00FF00FF);
	n = (n & 0x0000FFFF) + ((n >> 16)& 0x0000FFFF);
	return n;
}

// needed to break circular include Chunk<->ChunkGlState :<
struct ChunkConstants {
	using Key = u64;
	using Pos = i32;
	using ProtPos = i32;

	using ProtGid = u32;

	static constexpr sz_t size = CHUNK_CONSTANTS_SIZE;
	static constexpr sz_t protectionAreaSize = 16;
	static constexpr u8 pxTexNumChannels = 4; // RGBA

	// pc**2 = dimensions of the protection array for chunks
	static constexpr sz_t pc = size / protectionAreaSize;

	static constexpr u32 pcShift  = popc(pc - 1);
	static constexpr u32 pSizeShift  = popc(protectionAreaSize - 1);
	static constexpr u32 posShift = popc(size - 1);

	// split one chunk to protection cells
	// with specific per-world, or general uvias roles
	using ProtTexture = std::array<ProtGid, pc * pc>;

	constexpr static Key key(Pos x, Pos y) {
		union {
			struct {
				Pos x;
				Pos y;
			} p;
			Key xy;
		} k = {{x, y}};
		return k.xy;
	}

	static_assert((size & (size - 1)) == 0,
		"Chunk size must be a power of 2");

	static_assert((protectionAreaSize & (protectionAreaSize - 1)) == 0,
		"Chunk protectionAreaSize must be a power of 2");

	static_assert((size % protectionAreaSize) == 0,
		"Chunk size must be divisible by protectionAreaSize");

	static_assert((pc & (pc - 1)) == 0,
		"size / protectionAreaSize must result in a power of 2");
};

