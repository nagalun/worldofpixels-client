#pragma once

#include <array>
#include <memory>

#include "explints.hpp"
#include "color.hpp"
#include "PngImage.hpp"

constexpr u8 popc(u32 n) {
	n = (n & 0x55555555) + ((n >> 1) & 0x55555555);
	n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
	n = (n & 0x0F0F0F0F) + ((n >> 4) & 0x0F0F0F0F);
	n = (n & 0x00FF00FF) + ((n >> 8) & 0x00FF00FF);
	n = (n & 0x0000FFFF) + ((n >> 16)& 0x0000FFFF);
	return n;
}

class World;

class Chunk {
public:
	using Key = u64;
	using Pos = i32;
	using ProtPos = i32;

	static constexpr sz_t size = 512;
	static constexpr sz_t protectionAreaSize = 16;

	// pc**2 = dimensions of the protection array for chunks
	static constexpr sz_t pc = size / protectionAreaSize;

	static constexpr u32 pcShift  = popc(pc - 1);
	static constexpr u32 pSizeShift  = popc(protectionAreaSize - 1);
	static constexpr u32 posShift = popc(size - 1);

private:
	World& w;
	const Pos x;
	const Pos y;
	PngImage data;
	std::array<u32, pc * pc> protectionData; // split one chunk to protection cells
	// with specific per-world, or general uvias roles
	std::unique_ptr<void, void(*)(void *)> loaderRequest;
	//u32 texUnit;
	mutable u32 texHdl;
	bool canUnload;
	bool protectionsLoaded;
	u8 downscaling;

public:
	Chunk(Pos x, Pos y, World&);

	~Chunk();

	Pos getX() const;
	Pos getY() const;

	bool setPixel(u16 x, u16 y, RGB_u);
	RGB_u getPixel(u16 x, u16 y) const;

	const u8 * getData() const;

	void setProtectionGid(ProtPos x, ProtPos y, u32 gid);
	u32 getProtectionGid(ProtPos x, ProtPos y) const;

	bool isReady() const;
	bool shouldUnload() const;
	void preventUnloading(bool);

	u32 getGlTexture() const;
	void deleteTexture();

	static Key key(Pos, Pos);

private:
	static void loadCompleted(unsigned, void * e, void * buf, unsigned len);
	static void loadFailed(unsigned, void * e, int code, const char * err);
};
