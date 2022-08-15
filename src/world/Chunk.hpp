#pragma once

#include <util/color.hpp>
#include <util/explints.hpp>
#include <util/PngImage.hpp>
#include <array>
#include <memory>

#include <gl/ChunkGlState.hpp>
#include <world/ChunkConstants.hpp>

class World;

class Chunk : public ChunkConstants {
	World& w;
	const Pos x;
	const Pos y;

	ProtTexture protectionData;
	std::unique_ptr<void, void(*)(void *)> loaderRequest;

	ChunkGlState glst;
	bool canUnload;
	u8 downscaling;

public:
	Chunk(Pos x, Pos y, World&);
	~Chunk();

	Pos getX() const;
	Pos getY() const;

	bool setPixel(u16 x, u16 y, RGB_u, bool alphaBlending);
	RGB_u getPixel(u16 x, u16 y) const;

	const u8 * getData() const;

	void setProtectionGid(ProtPos x, ProtPos y, ProtGid gid);
	ProtGid getProtectionGid(ProtPos x, ProtPos y) const;

	bool isReady() const;
	bool shouldUnload() const;
	void preventUnloading(bool);

	ChunkGlState& getGlState();
	const ChunkGlState& getGlState() const;

private:
	static void loadCompleted(unsigned, void * e, void * buf, unsigned len);
	static void loadFailed(unsigned, void * e, int code, const char * err);
};
